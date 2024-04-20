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
server_find_client(struct server *server, int fd)
{
    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct client *client = &server->clients[i];

        if (!client_is_closed(client)) {
            continue;
        }

        if (client_get_fd(client) == fd) {
            return client;
        }
    }

    return NULL;
}

static struct client *
server_alloc_client(struct server *server, int fd)
{
    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct client *client = &server->clients[i];

        if (client_is_closed(client)) {
            client_open(client, fd);
            return client;
        }
    }

    return NULL;
}

static void
server_close_client(struct server *server, int fd)
{
    client_close(server_find_client(server, fd));
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

    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        client_init(&server->clients[i]);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    error = bind(server->fd, (struct sockaddr *)&addr, sizeof(addr));
    
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

    // TODO Handle error.

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
    nfds_t size;

    assert(server);
    assert(fdarray);
    assert(fdarray_size);

    fdarray[0].fd = server->fd;
    fdarray[0].events = POLLIN;

    size = 1;

    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct client *client = &server->clients[i];

        if (!client_is_closed(client)) {
            fdarray[size].fd =  client->fd;
            fdarray[size].events = POLLIN;
            size++;
        }
    }
}

static int
server_handle_revents(struct server *server)
{
    return server_accept_client(server);
}

static int
server_handle_client_revents(struct server *server, int fd)
{
    return client_process(server_find_client(server, fd));
}

static int
server_process(struct server *server)
{
    struct pollfd fdarray[ARRAY_SIZE(server->clients) + 1];
    nfds_t fdarray_size;
    int nb_events;
    int error = 0;

    server_build_fdarray(server, fdarray, &fdarray_size);

    nb_events = poll(fdarray, fdarray_size, -1);
    
    if (nb_events == -1) {
        return -1;
    }

    for (nfds_t i = 0; i < fdarray_size; i++) {
        const struct pollfd *pollfd = &fdarray[i];

        if (pollfd->revents == 0) {
            continue;
        }

        if (pollfd->fd == server->fd) {
            error = server_handle_revents(server);
        } else {
            error = server_handle_client_revents(server, pollfd->fd);

            if (error != 0) {
                server_close_client(server, pollfd->fd);
            }
        }
    }

    return 0;
}

void
server_poll(struct server *server)
{
    int error;

    do {
        error = server_process(server);
    } while (!error);
}
