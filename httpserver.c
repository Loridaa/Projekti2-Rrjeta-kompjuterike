#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define HTTP_PORT 8081
#define MAX_CLIENTS 4

typedef struct{
    struct sockaddr_in addr;
    time_t last_active;
    int is_admin;
    char ip[INET_ADDRSTRLEN];
} Client;

extern Client clients[MAX_CLIENTS];
extern int client_count;
extern pthread_mutex_t clients_mutex;

extern char messages[200][512];
extern int message_count;
extern pthread_mutex_t msg_mutex;

static void escape_json(const char *input, char *output, size_t outsz){
    size_t j = 0;
    for(size_t i = 0; input[i] && j + 3 < outsz; i++){
        char c = input[i];
        if(c == '"' || c == '\\'){
            output[j++] = '\\';
            output[j++] = c;
        }else if(c == '\n'){
            output[j++] = '\\';
            output[j++] = 'n';
        }else if(c == '\r'){
            output[j++] = '\\';
            output[j++] = 'r';
        }else if(c == '\t'){
            output[j++] = '\\';
            output[j++] = 't';
        }else{
            output[j++] = c;
        }
    }
    output[j] = '\0';
}

void *http_server(void *arg){
    (void)arg;

    int http_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(http_sock < 0){
        printf("[http] socket deshtoi\n");
        return NULL;
    } 

    int opt = 1;
    setsockopt(http_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(http_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        printf("[http] bind deshtoi\n");
        close(http_sock);
        return NULL;
    }

    if(listen(http_sock, 5) < 0){
        printf("[http] listen deshtoi\n");
        close(http_sock);
        return NULL;    
    }

    printf("[http] serveri i statistikave aktiv ne port %d -> GET /stats\n", HTTP_PORT);

    while(1){
        int client = accept(http_sock, NULL, NULL);
        if(client < 0) continue;

        char req[1024] = {0};
        int received = recv(client, req, sizeof(req) - 1, 0);
        if(received <= 0){
            close(client);
            continue;
        }
        req[received] = '\0';

        if(strncmp(req, "GET /stats", 10) != 0){
            const char *not_found = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "404 endpoint nuk ekziston. perdor GET /stats";
            
            send(client, not_found, strlen(not_found), 0);
            close(client);
            continue;
        }

        char body[65536];

        pthread_mutex_lock(&clients_mutex);
        pthread_mutex_lock(&msg_mutex);

        int pos = snprintf(body, sizeof(body),
            "{\n"
            "  \"klientet_aktiv\": %d, \n"
            "  \"max_klientet\": %d,\n"
            "  \"numri_mesazheve\": %d,\n"
            "  \"ip_adresat\": [",
            client_count, MAX_CLIENTS, message_count
        );

        for(int i = 0; i < client_count && pos < (int)sizeof(body); i++){
            pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
                            "%s\"%s\"", (i ? ", " : ""), clients[i].ip); 
        }

        pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "],\n  \"mesazhet\": [");

        for (int i = 0; i < message_count && pos < (int)sizeof(body); i++) {
            char escaped[1024];
            escape_json(messages[i], escaped, sizeof(escaped));
            pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
                            "%s\"%s\"", (i ? ", " : ""), escaped);
        }

        snprintf(body + pos, sizeof(body) - (size_t)pos, "]\n}");

        pthread_mutex_unlock(&msg_mutex);
        pthread_mutex_unlock(&clients_mutex);

        char response[70000];
        snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Connection: close\r\n\r\n%s", body
        );

        send(client, response, strlen(response), 0);
        close(client);
    }
    close(http_sock);
    return NULL;
}