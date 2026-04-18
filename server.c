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

/* funksione nga httpserver.c */
void *http_server(void *arg);

/* funksione nga filemanage*/
void cmd_list(char *output, size_t outsz);
void cmd_read(const char *filename, char *output, size_t outsz);
void cmd_delete(const char *filename, char *output, size_t outsz);
void cmd_search(const char *keyword, char *output, size_t outsz);
void cmd_info(const char *filename, char *output, size_t outsz);
void cmd_upload(const char *filename, const char *content, char *output, size_t outsz);
void cmd_download(const char *filename, char *output, size_t outsz);