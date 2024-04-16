#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "macro.h"
#include "server.h"

#define SERVER_PORT                1026
#define SERVER_BACKLOG_SIZE        2

static struct client *
server_retrieve_client(struct server *server, int fd)
{
    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        
        if (server->clients[i].fd == fd){
            return &server->clients[i];
        }
    }
    return NULL;
}

static struct client *
server_alloc_client(struct server *server, int fd)
{
    if (fd < 0) {
        return NULL;
    }

    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct client *client;
        client = &server->clients[i];

        if (client_is_available(client)) {
            client_init(client, fd);
            return client;
        }
    }

    return NULL;
}

static void
server_close_client(struct server *server, const struct pollfd pollfd)
{
    struct client *client;

    client = server_retrieve_client(server, pollfd.fd);

    if (client != NULL) {
        client_close(client);
    }
}

int
server_init(struct server *server)
{
    int error = 0;
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
   
    if (fd == -1) {
        perror("socket creation error:");
        return -1;  
    }

    server->fd = fd;
    memset(server->clients, -1, SERVER_MAX_NR_CLIENT * sizeof(struct client));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    error = bind(server->fd, (struct sockaddr *) &addr, sizeof(addr));
    
    if (error == -1) {
        goto fail;
    }

    error = listen(server->fd, SERVER_BACKLOG_SIZE);
    
    if (error == -1) {
        goto fail;
    }

    return 0;

fail :
    perror("error:");
    close(server->fd);
    return -1;
}

static int
server_accept_client(struct server *server)
{
    int fd;
    struct client *client;
    struct sockaddr_in client_addr;
    socklen_t  client_addr_size;
    
    client_addr_size = sizeof(client_addr);
    fd = accept(server->fd, (struct sockaddr *) &client_addr, &client_addr_size);
    
    client = server_alloc_client(server, fd);

    if (client) {
        return 0;
    } else {
        return -1;
    }
}

static void
server_build_fdarray(struct server *server, struct pollfd *fdarray, nfds_t *fdarray_size)
{
    fdarray[*(fdarray_size)].fd =  server->fd;
    fdarray[*(fdarray_size)].events =  POLLIN;
    *(fdarray_size) = 1;

    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct client *client;
        client = &server->clients[i];

        if (client_is_busy(client)) {
            fdarray[*(fdarray_size)].fd =  client->fd;
            fdarray[*(fdarray_size)].events =  POLLIN;
            *(fdarray_size) = *(fdarray_size) + 1;
        }
    }
}

static int
server_handle_revents(struct server *server, short revent)
{
    if (revent == POLLIN) {
        return server_accept_client(server);
    } else {
        return 0;
    }
}

static int
server_handle_client_revents(struct server *server, const struct pollfd pollfd)
{
    int error = 0;
    struct client *client = NULL;
    
    client = server_retrieve_client(server, pollfd.fd);

    if (client == NULL) {
        error = -1;
    }

    if (pollfd.revents == POLLIN) {
        error = client_receive(client);
    }

    return error; 
}

int
server_poll(struct server *server)
{
    int nb_events;
    int error = 0;
 
    while(1) {

        struct pollfd fdarray[ARRAY_SIZE(server->clients)+1];
        nfds_t fdarray_size;
        
        server_build_fdarray(server, fdarray, &fdarray_size);

        nb_events = poll(fdarray, fdarray_size, -1);
        
        if (nb_events == -1){
            error = 1;
        }

        for (size_t i = 0; i < fdarray_size; i++) {
            if (fdarray[i].revents == 0) {
                continue;
            }

            if (fdarray[i].fd == server->fd) {
                error = server_handle_revents(server, fdarray[i].revents);
            } else {
                error = server_handle_client_revents(server, fdarray[i]);
            }

            if (error != 0) {
                server_close_client(server, fdarray[i]);
            }       
        }
    }
}