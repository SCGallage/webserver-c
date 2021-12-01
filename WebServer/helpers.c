//#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define SOCKET int
#define MAX_REQUEST_SIZE 2047
#define ISINVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)

#define BSIZE 1024

#include "../LRU/LinearProbe.c"

struct client_info
{
    socklen_t address_length;
    struct sockaddr_storage address;
    SOCKET socket;
    char request[MAX_REQUEST_SIZE];
    int received;
    struct client_info *next;
};

char buffer[BSIZE];
bool initialize = false;
// const char* get_content_type(const char* path);
// struct client_info* get_client(SOCKET s);
// void drop_client(struct client_info *client);
// const char* get_client_address(struct client_info *ci);
// fd_set wait_on_clients(SOCKET server);
// void send_400(struct client_info *client);
// void send_404(struct client_info *client);
// void serve_resource(struct client_info *client, const char *path);
static struct client_info *clients = 0;
off_t fileSize(char* filePath);

const char* get_content_type(const char* path) {
    const char *last_dot = strrchr(path, '.');
    if (last_dot)
    {
        if (strcmp(last_dot, ".css") == 0) return "text/css";
        if (strcmp(last_dot, ".csv") == 0) return "text/csv";
        if (strcmp(last_dot, ".gif") == 0) return "image/gif";
        if (strcmp(last_dot, ".htm") == 0) return "text/html";
        if (strcmp(last_dot, ".html") == 0) return "text/html";
        if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
        if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".js") == 0) return "application/javascript";
        if (strcmp(last_dot, ".png") == 0) return "image/png";
        if (strcmp(last_dot, ".txt") == 0) return "text/plain";
    }
    return "application/octet-stream";
    
}

struct client_info* get_client(SOCKET s) {
    struct client_info *ci = clients;

    while (ci)
    {
        if (ci->socket == s)
            break;
        ci = ci->next;   
    }
    
    if (ci) return ci;

    struct client_info *n = (struct client_info*) calloc(1, sizeof(struct client_info));

    if (!n)
    {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    n->address_length = sizeof(n->address);
    n->next = clients;
    clients = n;
    return n;
    
}

void drop_client(struct client_info *client) {

    CLOSESOCKET(client->socket);

    struct client_info **p = &clients;

    while (*p)
    {
        if (*p == client) {
            *p = client->next;
            free(client);
            return;
        }
        p = &(*p)->next;
    }
    
    fprintf(stderr, "drop_client not found.\n");
    exit(1);
}

const char* get_client_address(struct client_info *ci) {
    static char address_buffer[100];
    getnameinfo((struct sockaddr*)&ci->address, ci->address_length,
    address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    return address_buffer;
}

fd_set wait_on_clients(SOCKET server) {
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(server, &reads);
    SOCKET max_socket = server;

    struct client_info *ci = clients;
    
    while (ci)
    {
        //printf("Waiting..\n");
        FD_SET(ci->socket, &reads);
        if (ci->socket > max_socket)
            max_socket = ci->socket;
        ci = ci->next;
    }   

    if (select(max_socket+1, &reads, 0, 0, 0) < 0)
    {
        fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }
    //printf("Finished..\n");
    return reads;
}

void send_400(struct client_info *client) {
    const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
        "Connection: close\r\n"
        "Content-Length: 11\r\n\r\nBad Request";
    send(client->socket, c400, strlen(c400), 0);
    drop_client(client);
}

void send_404(struct client_info *client) {
    const char *c404 = "HTTP/1.1 404 Not Found\r\n"
        "Connection: close\r\n"
        "Content-Length: 9\r\n\r\nNot Found";
    send(client->socket, c404, strlen(c404), 0);
    drop_client(client);
}

void serve_resource(struct client_info *client, const char *path) {
    printf("serve resource %s %s\n", get_client_address(client), path);
    
    if (!initialize)
    {
        initializeHashTable(0.8);
        initialize = true;
    }
    
    if (!strcmp(path, "/")) path = "/index.html";

    if (strlen(path) > 100)
    {
        send_400(client);
        return;
    }

    if (strstr(path, ".."))
    {
        send_404(client);
        return;
    }
    
    char full_path[128];
    sprintf(full_path, "public%s", path);

    printf("Bool: %d\n", keyExists(full_path));

    const char *ct = get_content_type(full_path);

    if (keyExists(full_path))
    {
        //printf("reached!\n");
        struct node *existing = get(full_path);
        printf("Found: %s\n", existing->data);

        sprintf(buffer, "HTTP/1.1 200 OK\r\n");
        send(client->socket, buffer, strlen(buffer), 0);

        sprintf(buffer, "Connection: close\r\n");
        send(client->socket, buffer, strlen(buffer), 0);

        sprintf(buffer, "Content-Length: %u\r\n", strlen(existing->data));
        send(client->socket, buffer, strlen(buffer), 0);

        sprintf(buffer, "Content-Type: %s\r\n", ct);
        send(client->socket, buffer, strlen(buffer), 0);
        
        sprintf(buffer, "\r\n");
        send(client->socket, buffer, strlen(buffer), 0);

        send(client->socket, existing->data, strlen(existing->data), 0);
    } else{
        //printf("Reached!\n");
        FILE *fp = fopen(full_path, "rb");

        off_t size = fileSize(full_path);
        printf("Size of Buffer: %d\n", sizeof(buffer));

        if (!fp)
        {
            send_404(client);
            return;
        }
        
        fseek(fp, 0L, SEEK_END);
        size_t cl = ftell(fp);
        rewind(fp);

        sprintf(buffer, "HTTP/1.1 200 OK\r\n");
        send(client->socket, buffer, strlen(buffer), 0);

        sprintf(buffer, "Connection: close\r\n");
        send(client->socket, buffer, strlen(buffer), 0);

        sprintf(buffer, "Content-Length: %u\r\n", cl);
        send(client->socket, buffer, strlen(buffer), 0);

        sprintf(buffer, "Content-Type: %s\r\n", ct);
        send(client->socket, buffer, strlen(buffer), 0);
        
        sprintf(buffer, "\r\n");
        send(client->socket, buffer, strlen(buffer), 0);

        int r = fread(buffer, 1, BSIZE, fp);
        while (r)
        {
            send(client->socket, buffer, r, 0);
            r = fread(buffer, 1, BSIZE, fp);
        }

        if (size <= BSIZE){
            //printf("Smaller!\n");
            insert(full_path, buffer);
        }

        fclose(fp);
    }

    drop_client(client);
    
}

off_t fileSize(char* filePath) {
    struct stat st;
    stat(filePath, &st);
    printf("File Size: %ld\n", st.st_size);
    return st.st_size;
}