#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define SOCKET int
#define MAX_REQUEST_SIZE 2047
#define ISINVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)


// #include "helpers.h"
#include "helpers.c"

//const char* get_content_type(const char* path);
SOCKET create_socket(const char* host, const char* port);

SOCKET create_socket(const char* host, const char* port) {

    printf("Configuring local address.........\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(host, port, &hints, &bind_address);

    printf("Creating socket.......\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

    if (!ISINVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed. (%d)", GETSOCKETERRNO());
        exit(1);
    }

    printf("Binding socket to local address.....\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)", GETSOCKETERRNO());
        exit(1);
    }
    freeaddrinfo(bind_address);

    printf("Listening.....\n");
    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "listen() failed. (%d)", GETSOCKETERRNO());
        exit(1);
    }

    return socket_listen;

}

int main() {
    SOCKET server = create_socket(0, "8080");
    
    while (1)
    {
        fd_set reads;
        reads = wait_on_clients(server);
        if (FD_ISSET(server, &reads))
        {
            struct client_info *client = get_client(-1);

            client->socket = accept(server, (struct sockaddr*) &(client->address), &(client->address_length));

            if (!ISINVALIDSOCKET(client->socket))
            {
                fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
                exit(1);
            }

            printf("New connection from %s.\n", get_client_address(client));
        }
        struct client_info *client = clients;
        // printf("%p\n", client);
        
        while (client)
        {
            // printf("Hello Man!\n");
            struct client_info *next = client->next;
            if(FD_ISSET(client->socket, &reads)) {
                if(MAX_REQUEST_SIZE == client->received) {
                    send_400(client);
                    continue;
                }

                int r = recv(client->socket, 
                    client->request + client->received, 
                    MAX_REQUEST_SIZE - client->received, 0);
                
                if (r < 1){
                    printf("Unexpected disconnect from %s.\n", get_client_address(client));
                    drop_client(client);
                } else {
                    client->received += r;
                    client->request[client->received] = 0;

                    char *q = strstr(client->request, "\r\n\r\n");
                    if (q)
                    {
                        if (strncmp("GET /", client->request, 5)){
                            send_400(client);
                        } else {
                            char *path = client->request + 4;
                            char *end_path = strstr(path, " ");
                            if (!end_path){
                                send_400(client);
                            } else {
                                *end_path = 0;
                                serve_resource(client, path);
                            }
                        }
                    }
                }
            }
            client = next;
        }
        //printf("Reached!");
    }

    printf("\nClosing socket...\n");
    CLOSESOCKET(server);

    printf("Finished.\n");
    return 0;
}