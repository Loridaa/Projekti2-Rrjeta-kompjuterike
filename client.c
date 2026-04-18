#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "menu_ui.h"


#define SERVER_IP   "127.0.0.1"
#define PORT        9000
#define BUFFER_SIZE 1048576


int krijo_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket creation failed"); exit(EXIT_FAILURE); }

   
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return sockfd;
}


void komunikimi_baze(int sockfd, struct sockaddr_in servaddr, const char *mesazhi,
                     char *result, size_t resultsz) {
    char buffer[BUFFER_SIZE];
    unsigned int len = sizeof(servaddr);

    sendto(sockfd, mesazhi, strlen(mesazhi), 0,
           (const struct sockaddr *)&servaddr, len);

    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
    if (n < 0) {
        snprintf(result, resultsz, "[gabim] serveri nuk u pergjigj (timeout)");
        return;
    }
    buffer[n] = '\0';

  
    if (strncmp(buffer, "FILE_DATA:", 10) == 0) {
        char *data_start = buffer + 10;
        char *filename   = strtok(data_start, ":");
        char *content    = strtok(NULL, "");

        if (!filename || !content) {
            snprintf(result, resultsz, "[gabim] formati i file-it nga serveri eshte i gabuar.");
            return;
        }

        char save_name[300];
        snprintf(save_name, sizeof(save_name), "shkarkuar_%s", filename);

        FILE *f = fopen(save_name, "w");
        if (!f) {
            snprintf(result, resultsz, "[gabim] nuk u krijua dot file lokal.");
            return;
        }
        fprintf(f, "%s", content);
        fclose(f);
        snprintf(result, resultsz, "[ok] file u ruajt si '%s'", save_name);

    } else {
        snprintf(result, resultsz, "[serveri]: %s", buffer);
    }

    result[resultsz - 1] = '\0';
}


void menuja_klientit(int sockfd, struct sockaddr_in servaddr, int isAdmin,
                     const char *initial_status) {
    char line[1024];
    char *full_request = malloc(BUFFER_SIZE);
    char *last_result = malloc(BUFFER_SIZE);
    if (!full_request || !last_result) {
        printf("[gabim] malloc deshtoi\n");
        free(full_request);
        free(last_result);
        return;
    }

    if (initial_status && initial_status[0] != '\0') {
        snprintf(last_result, BUFFER_SIZE, "[serveri]: %s", initial_status);
    } else {
        last_result[0] = '\0';
    }

    while (1) {
        ui_render_dashboard(SERVER_IP, PORT, isAdmin, last_result);

        if (fgets(line, sizeof(line), stdin) == NULL) break;
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) continue;

        if (strcmp(line, "/exit") == 0) break;

        if (strcmp(line, "/help") == 0) {
            snprintf(last_result, BUFFER_SIZE, "[info] menu-ja eshte e shfaqur lart.");
            continue;
        }

      
        int is_write_cmd = (strncmp(line, "/delete",   7) == 0 ||
                            strncmp(line, "/upload",   7) == 0 ||
                            strncmp(line, "/download", 9) == 0);

        if (!isAdmin && is_write_cmd) {
            snprintf(last_result, BUFFER_SIZE,
                     "[nuk lejohet] vetem admin mund te perdore kete komande.");
            continue;
        }

        if (strcmp(line, "/upload") == 0) {
            snprintf(last_result, BUFFER_SIZE, "perdorimi: /upload <filename>");
            continue;
        }

        
        if (strncmp(line, "/upload ", 8) == 0) {
            char filename[256];
            if (sscanf(line, "/upload %255s", filename) != 1) {
                snprintf(last_result, BUFFER_SIZE, "perdorimi: /upload <filename>");
                continue;
            }

            FILE *file = fopen(filename, "r");
            if (!file) {
                snprintf(last_result, BUFFER_SIZE,
                         "[gabim] file '%s' nuk ekziston lokalisht.", filename);
                continue;
            }

            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            rewind(file);

            if (fsize > BUFFER_SIZE / 2) {
                snprintf(last_result, BUFFER_SIZE,
                         "[gabim] file shume i madh per tu derguar.");
                fclose(file); continue;
            }

            char *content = malloc(fsize + 1);
            if (!content) {
                fclose(file);
                snprintf(last_result, BUFFER_SIZE,
                         "[gabim] nuk ka memorie te mjaftueshme.");
                continue;
            }
            fread(content, 1, fsize, file);
            content[fsize] = '\0';
            fclose(file);

            
            snprintf(full_request, BUFFER_SIZE, "PRIORITY_HIGH|/upload %s %s",
                     filename, content);
            free(content);

            komunikimi_baze(sockfd, servaddr, full_request,
                            last_result, BUFFER_SIZE);
            continue;
        }

        
        if (isAdmin)
            snprintf(full_request, BUFFER_SIZE, "PRIORITY_HIGH|%s", line);
        else
            snprintf(full_request, BUFFER_SIZE, "PRIORITY_LOW|%s",  line);

        komunikimi_baze(sockfd, servaddr, full_request,
                        last_result, BUFFER_SIZE);
    }

    ui_clear_screen();
    free(full_request);
    free(last_result);
}
int main() {
    int sockfd = krijo_socket();

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("klienti udp  ->  %s:%d\n", SERVER_IP, PORT);

    int roli;
    printf("zgjidh rolin  (1 = admin,  2 = user): ");
    if (scanf("%d", &roli) != 1) {
        printf("[gabim] input i pavlefshem.\n");
        close(sockfd); return 1;
    }
    getchar();


    char init_msg[64];
    snprintf(init_msg, sizeof(init_msg), "%s",
             (roli == 1) ? "admin123" : "user_connect");
    sendto(sockfd, init_msg, strlen(init_msg), 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr));

  
    char ack[256] = {0};
    int ack_n = recvfrom(sockfd, ack, sizeof(ack)-1, 0, NULL, NULL);
    if (ack_n < 0) {
        perror("[gabim] serveri nuk u pergjigj ne lidhjen fillestare");
        close(sockfd);
        return 1;
    }
    ack[ack_n] = '\0';

    menuja_klientit(sockfd, servaddr, (roli == 1), ack);

    close(sockfd);
    printf("[info] u shkepute nga serveri.\n");
    return 0;
}
