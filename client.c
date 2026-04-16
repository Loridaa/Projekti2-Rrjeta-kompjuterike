#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


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


void komunikimi_baze(int sockfd, struct sockaddr_in servaddr, char *mesazhi) {
    char buffer[BUFFER_SIZE];
    unsigned int len = sizeof(servaddr);

    sendto(sockfd, mesazhi, strlen(mesazhi), 0,
           (const struct sockaddr *)&servaddr, len);

    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
    if (n < 0) {
        perror("[gabim] serveri nuk u pergjigj (timeout)");
        return;
    }
    buffer[n] = '\0';

  
    if (strncmp(buffer, "FILE_DATA:", 10) == 0) {
        char *data_start = buffer + 10;
        char *filename   = strtok(data_start, ":");
        char *content    = strtok(NULL, "");

        if (!filename || !content) {
            printf("[gabim] formati i file-it nga serveri eshte i gabuar.\n");
            return;
        }

        char save_name[300];
        snprintf(save_name, sizeof(save_name), "shkarkuar_%s", filename);

        FILE *f = fopen(save_name, "w");
        if (!f) { perror("[gabim] nuk u krijua dot file lokal"); return; }
        fprintf(f, "%s", content);
        fclose(f);
        printf("[ok] file u ruajt si '%s'\n", save_name);

    } else {
        printf("[serveri]: %s\n", buffer);
    }
}