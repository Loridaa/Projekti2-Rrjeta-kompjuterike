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


void menuja_klientit(int sockfd, struct sockaddr_in servaddr, int isAdmin) {
    char line[1024];
    char *full_request = malloc(BUFFER_SIZE);
    if (!full_request) { printf("[gabim] malloc deshtoi\n"); return; }

    while (1) {
        printf("\nmenu [%s]\n", isAdmin ? "admin" : "user");
        printf("komanda (/help per ndihme): ");

        if (fgets(line, sizeof(line), stdin) == NULL) break;
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) continue;

        if (strcmp(line, "/exit") == 0) break;

        if (strcmp(line, "/help") == 0) {
            printf("\nkomandat e disponueshme:\n");
            printf("  /list                listoji file-t ne server\n");
            printf("  /read   <filename>   lexo permbajtjen e nje file-i\n");
            printf("  /search <fjala>      kerko file sipas emrit\n");
            printf("  /info   <filename>   shfaq madhesine dhe datat e file-it\n");
            if (isAdmin) {
                printf("  /upload   <filename> dergo nje file ne server\n");
                printf("  /download <filename> shkarko nje file nga serveri\n");
                printf("  /delete   <filename> fshi nje file ne server\n");
            }
            printf("  /exit                shkepute nga serveri\n");
            continue;
        }

      
        int is_write_cmd = (strncmp(line, "/delete",   7) == 0 ||
                            strncmp(line, "/upload",   7) == 0 ||
                            strncmp(line, "/download", 9) == 0);

        if (!isAdmin && is_write_cmd) {
            printf("[nuk lejohet] vetem admin mund te perdore kete komande.\n");
            continue;
        }

        
        if (strncmp(line, "/upload ", 8) == 0) {
            char filename[256];
            if (sscanf(line, "/upload %255s", filename) != 1) {
                printf("perdorimi: /upload <filename>\n");
                continue;
            }

            FILE *file = fopen(filename, "r");
            if (!file) {
                printf("[gabim] file '%s' nuk ekziston lokalisht.\n", filename);
                continue;
            }

            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            rewind(file);

            if (fsize > BUFFER_SIZE / 2) {
                printf("[gabim] file shume i madh per tu derguar.\n");
                fclose(file); continue;
            }

            char *content = malloc(fsize + 1);
            if (!content) { fclose(file); continue; }
            fread(content, 1, fsize, file);
            content[fsize] = '\0';
            fclose(file);

            /* formati qe pret serveri: PRIORITY_HIGH|/upload <emri> <permbajtja> */
            snprintf(full_request, BUFFER_SIZE, "PRIORITY_HIGH|/upload %s %s",
                     filename, content);
            free(content);

            printf("[info] duke derguar '%s' (%ld bytes)...\n", filename, fsize);
            komunikimi_baze(sockfd, servaddr, full_request);
            continue;
        }

        
        if (isAdmin)
            snprintf(full_request, BUFFER_SIZE, "PRIORITY_HIGH|%s", line);
        else
            snprintf(full_request, BUFFER_SIZE, "PRIORITY_LOW|%s",  line);

        komunikimi_baze(sockfd, servaddr, full_request);
    }

    free(full_request);
}