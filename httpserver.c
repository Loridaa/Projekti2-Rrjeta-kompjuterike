#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

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

void *http_server(void *arg){
    (void)arg;

    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
        printf("[http] WSAStartup deshtoi\n");
        return NULL;
    }

    SOCKET http_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(http_sock == INVALID_SOCKET){
        printf("[http] socket deshtoi\n");
        WSACleanup();
        return NULL;
    } 

    BOOL opt = 1;
    setsockopt(http_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(http_sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR){
        printf("[http] bind deshtoi\n");
        closesocket(http_sock);
        WSACleanup();
        return NULL;
    }

    if(listen(http_sock, 5) == SOCKET_ERROR){
        printf("[http] listen deshtoi\n");
        closesocket(http_sock);
        WSACleanup();
        return NULL;    
    }

    printf("[http] serveri i statistikave aktiv ne port %d -> GET /stats\n");

    while(1){
        SOCKET client = accept(http_sock, NULL, NULL);
        if(client == INVALID_SOCKET) continue;

        char req[1024] = {0};
        recv(client, req, sizeof(req) - 1, 0);

        if(strncmp(req, "GET /stats", 10) != 0){
            const char *not_found = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "404 endpoint nuk ekziston. perdor GET /stats";
            
            send(client, not_found, (int)strlen(not_found), 0);
            closesocket(client);
            continue;
        }

        char body[65536];

        pthread_mutex_lock(&clients_mutex);
        pthread_mutex_lock(&msg_mutex);

        int pos = snprintf(body, sizeof(body),
            "{\n}"
            "  \"klinetet_aktiv\": %d, \n"
            "  \"max_klientet\": %d,\n"
            "  \"numri_mesazheve\": %d,\n"
            "  \"ip_adresat\": [",
            client_count, MAX_CLIENTS, message_count
        );

        for(int i = 0; i < client_count && pos < (int)sizeof(body); i++){
            pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
                            "%s\"%s\"", (i ? ", " : ""), clients[i].ip); 
        }

        snprintf(body + pos, sizeof(body) - (size_t)pos, "]\n}");

        pthread_mutex_unlock(&msg_mutex);
        pthread_mutex_unlock(&clients_mutex);

        char response[70000];
        sprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Connection: close\r\n\r\n%s, body"
        );

        send(client, response, (int)strlen(response), 0);
        closesocket(client);
    }
    closesocket(http_sock);
    WSACleanup();
    return NULL;
}