#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <stddef.h>

#define UDP_PORT       9000
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