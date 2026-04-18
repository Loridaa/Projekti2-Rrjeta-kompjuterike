#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>

#define UDP_PORT       9000
#define SERVER_IP      "0.0.0.0"
#define MAX_CLIENTS    4
#define BUFFER_SIZE    1048576
#define TIMEOUT_SEC    30
#define ADMIN_PASSWORD "admin123"

/*
 * ruan te dhenat baze per secilin klient aktiv:
 */
typedef struct {
    struct sockaddr_in addr;
    time_t last_active;
    int    is_admin;
    char   ip[INET_ADDRSTRLEN];
} Client;

/*
 *  perdoren nga serveri udp edhe nga http serveri
 * per monitorim dhe statistika
 */
Client clients[MAX_CLIENTS];
int    client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char messages[200][512];
int  message_count = 0;
pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;

/* funksion nga httpserver.c */
void *http_server(void *arg);

/* funksione nga filemanage.c */
void cmd_list(char *output, size_t outsz);
void cmd_read(const char *filename, char *output, size_t outsz);
void cmd_delete(const char *filename, char *output, size_t outsz);
void cmd_search(const char *keyword, char *output, size_t outsz);
void cmd_info(const char *filename, char *output, size_t outsz);
void cmd_upload(const char *filename, const char *content, char *output, size_t outsz);
void cmd_download(const char *filename, char *output, size_t outsz);

/*
 * kerkon nje klient ekzistues sipas kombinimit IP + port
 */
int find_client(struct sockaddr_in *addr) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port        == addr->sin_port)
            return i;
    }
    return -1;
}

/*
 * shton nje klient te ri ne listen e klienteve aktiv
 */
void add_client(struct sockaddr_in addr, int is_admin) {
    if (client_count >= MAX_CLIENTS) return;
    clients[client_count].addr        = addr;
    clients[client_count].last_active = time(NULL);
    clients[client_count].is_admin    = is_admin;
    inet_ntop(AF_INET, &addr.sin_addr, clients[client_count].ip, INET_ADDRSTRLEN);
    client_count++;
    printf("[server] klient i ri: %s  (admin=%d)  gjithsej=%d\n",
           clients[client_count-1].ip, is_admin, client_count);
}

/*
 * heq klientet qe nuk kane derguar asnje mesazh per nje kohe te caktuar
 */
void remove_inactive_clients() {
    time_t now = time(NULL);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (difftime(now, clients[i].last_active) > TIMEOUT_SEC) {
            printf("[server] klienti %s u hoq per shkak te inaktivitetit.\n",
                   clients[i].ip);
            clients[i] = clients[client_count - 1];
            client_count--;
            i--;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/*
 * merret me procesimin e nje kerkese te vetme nga klienti.
 */
void process_request(int sockfd, struct sockaddr_in *client_addr,
                     socklen_t addr_len, char *buffer, int is_admin) {
    const size_t response_cap = BUFFER_SIZE;
    char *response = malloc(response_cap);
    if (!response) {
        const char *msg = "[server] gabim memorie.";
        sendto(sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)client_addr, addr_len);
        return;
    }
    response[0] = '\0';

    char *cmd = buffer;
    if      (strncmp(buffer, "PRIORITY_HIGH|", 14) == 0) cmd = buffer + 14;
    else if (strncmp(buffer, "PRIORITY_LOW|",  13) == 0) cmd = buffer + 13;

    if (strcmp(cmd, "/list") == 0) {
        cmd_list(response, response_cap);

    } else if (strncmp(cmd, "/read ", 6) == 0) {
        cmd_read(cmd + 6, response, response_cap);

    } else if (strncmp(cmd, "/delete ", 8) == 0) {
        if (!is_admin) strncpy(response, "[nuk lejohet] vetem admin.", response_cap - 1);
        else           cmd_delete(cmd + 8, response, response_cap);

    } else if (strncmp(cmd, "/search ", 8) == 0) {
        cmd_search(cmd + 8, response, response_cap);

    } else if (strncmp(cmd, "/info ", 6) == 0) {
        cmd_info(cmd + 6, response, response_cap);

    } else if (strncmp(cmd, "/download ", 10) == 0) {
        if (!is_admin) strncpy(response, "[nuk lejohet] vetem admin.", response_cap - 1);
        else           cmd_download(cmd + 10, response, response_cap);

    } else if (strncmp(cmd, "/upload ", 8) == 0) {
        if (!is_admin) {
            strncpy(response, "[nuk lejohet] vetem admin.", response_cap - 1);
        } else {
            char filename[256];
            if (sscanf(cmd + 8, "%255s", filename) == 1) {
                size_t offset = 8 + strlen(filename);
                if (cmd[offset] == ' ' && cmd[offset + 1] != '\0') {
                    cmd_upload(filename, cmd + offset + 1, response, response_cap);
                } else {
                    strncpy(response, "[gabim] permbajtja e file-it mungon.", response_cap - 1);
                }
            } else {
                strncpy(response, "[gabim] perdorimi: /upload <filename> <content>", response_cap - 1);
            }
        }
    } else {
        snprintf(response, response_cap, "[server] mesazhi u mor: %s", cmd);
    }

    response[response_cap - 1] = '\0';
        sendto(sockfd, response, strlen(response), 0,
            (struct sockaddr *)client_addr, addr_len);
        free(response);
}

/*
 *  main starton dy pjese:
 * 1) thread-in paralel te HTTP
 * 2) loop-in kryesor UDP
 */
int main() {
    mkdir("server_files", 0755);

    pthread_t http_thread;
    pthread_create(&http_thread, NULL, http_server, NULL);
    pthread_detach(http_thread);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

        printf("[server] udp po degjon ne %s:%d  (max %d klientet)\n",
            SERVER_IP, UDP_PORT, MAX_CLIENTS);

    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc");
        close(sockfd);
        return 1;
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) continue;
        buffer[n] = '\0';

        remove_inactive_clients();

        pthread_mutex_lock(&clients_mutex);
        int idx = find_client(&client_addr);

        /*
         * nese klienti eshte i ri, e regjistrojme si admin ose user.
         * nese serveri eshte plot, lidhja refuzohet.
         */
        if (idx == -1) {
            if (client_count >= MAX_CLIENTS) {
                const char *msg = "[server] serveri eshte plot. provoni me vone.";
                sendto(sockfd, msg, strlen(msg), 0,
                       (struct sockaddr *)&client_addr, addr_len);
                pthread_mutex_unlock(&clients_mutex);
                continue;
            }

            int is_admin = (strcmp(buffer, ADMIN_PASSWORD) == 0);
            add_client(client_addr, is_admin);
            idx = client_count - 1;

            const char *ack = is_admin
                ? "[server] u lidhet si admin."
                : "[server] u lidhet si user.";
            sendto(sockfd, ack, strlen(ack), 0,
                   (struct sockaddr *)&client_addr, addr_len);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }

        clients[idx].last_active = time(NULL);
        int is_admin = clients[idx].is_admin;
        pthread_mutex_unlock(&clients_mutex);

        /*
         * ruajme mesazhin ne log per monitorim
         */
        pthread_mutex_lock(&msg_mutex);
        if (message_count < 200) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
            char trunc[460];
            strncpy(trunc, buffer, sizeof(trunc)-1);
            trunc[sizeof(trunc)-1] = '\0';
            snprintf(messages[message_count++], 512, "[%s]: %s", ip, trunc);
        }
        pthread_mutex_unlock(&msg_mutex);

        printf("[udp] nga %s: %s\n", clients[idx].ip, buffer);

        /*
             user-at e zakonshem
         * marrin nje vonese te vogel, kurse admin trajtohet menjehere.
         */
        if (!is_admin) usleep(50000);

        process_request(sockfd, &client_addr, addr_len, buffer, is_admin);
    }

    free(buffer);
    close(sockfd);
    return 0;
}