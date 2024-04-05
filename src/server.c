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

static void
server_set_fd(struct server *server, int fd)
{
    server->fd = fd;
}

static int
server_convert_pollfd_index_to_client_index(size_t pollfd_index)
{
    int client_index;

    client_index = pollfd_index - 1;

    return client_index;
}

static struct client *
server_get_client(struct server *server, size_t index)
{
    assert(index > 0);
    assert(index < ARRAY_SIZE(server->clients));

    return &server->clients[index];
}


static bool
server_client_is_available(struct client *client)
{
    if (client->fd != 0) {
        return true;
    }
    else {
        return false;
    }
}

static struct client *
server_alloc_client(struct server *server, int fd)
{
    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct client *client;
        client = server_get_client(server, i);

        if (server_client_is_available(client)) {
            client_init(client, fd, i);
            return client;
        }
    }

    return NULL;
}

static void
server_invalid_pollfd_element(struct server *server, size_t pollfd_index)
{
    server->pollfds[pollfd_index].fd = -1;
    server->pollfds[pollfd_index].events = 0;
}

static void
server_invalid_client(struct client *client)
{
    client_set_fd(client, 0);
}

static void
server_close_client_connection(struct server *server, size_t pollfd_index)
{
    size_t client_index;
    struct client *client;
    
    client_index = server_convert_pollfd_index_to_client_index(pollfd_index);
    client = server_get_client(server, client_index);
    server_invalid_pollfd_element(server, pollfd_index);
    server_invalid_client(client);
    close(client->fd);
}

static void
server_add_pollfd_element(struct server *server, size_t pollfd_index, int fd)
{
    server->pollfds[pollfd_index].fd = fd;
    server->pollfds[pollfd_index].events = POLLIN;
}

void
server_tcp_ip_init(struct server *server)
{
    int ret;
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    server_set_fd(server, fd);

    if (server->fd == -1) {
        perror("socket creation error:"); 
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ret = bind(server->fd,(struct sockaddr *) &addr,sizeof(addr));
    
    if (ret == -1) {
        perror("Bind error:");
        close(server->fd);
        exit(EXIT_FAILURE);
    }

    ret=listen(server->fd, SERVER_BACKLOG_SIZE);
    
    if (ret == -1) {
        perror("Listen error:");
        close(server->fd);
        exit(EXIT_FAILURE);
    }

    server_add_pollfd_element(server, 0, fd);
}

static int
server_convert_client_index_to_pollfd_index(struct server *server, struct client client)
{
    size_t pollfd_index;

    assert(client.index + 1 < ARRAY_SIZE(server->pollfds));
    pollfd_index = client.index + 1;

    return pollfd_index;
}

static void
server_accept_client(struct server *server)
{
    int fd;
    struct client *client;
    struct sockaddr_in client_addr;
    socklen_t  client_addr_size;
    
    client_addr_size = sizeof(client_addr);
    fd = accept(server->fd, (struct sockaddr *) &client_addr, &client_addr_size);
    
    if (fd == -1) {
        goto fail;
    }

    client = server_alloc_client(server, fd);

    if (client) {
        size_t pollfd_index;
        pollfd_index = server_convert_client_index_to_pollfd_index(server, *client);
        server_add_pollfd_element(server, pollfd_index, fd);
    }
    else {
        goto fail;
    }

    return;

fail:
    perror("cannot accept new client :");
}

void 
server_poll(struct server *server){

    int nb_events;
    int client_index;
    int pollin_ret;
    struct client *client;

    while(1){
    
        nb_events = poll(server->pollfds, ARRAY_SIZE(server->pollfds), 1);
        
        if (nb_events == -1){
            goto fail;
        }

        for (size_t i = 0; i < ARRAY_SIZE(server->pollfds); i++) {

            if (server->pollfds[i].revents == 0) {
                continue;
            }

            if (server->pollfds[i].fd == server->fd) {

                if (server->pollfds[i].revents == POLLIN) {
                    server_accept_client(server);
                }
            }
            else{

                if (server->pollfds[i].revents == POLLIN) {
                    client_index = server_convert_pollfd_index_to_client_index(i);
                    client = server_get_client(server, client_index);
                    pollin_ret = client_pollin_behaviour(client);
                    
                    if (pollin_ret) {
                        server_close_client_connection(server, i);
                        goto fail;
                    }
                }
                else {
                    server_close_client_connection(server, i);
                    goto fail;
                }
            }        
        }
    }
    return;

    fail :
        perror("Poll error:");
        close(server->fd);
        exit(EXIT_FAILURE);
}