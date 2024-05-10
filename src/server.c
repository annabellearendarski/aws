#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "list.h"
#include "macro.h"
#include "server.h"

#define SERVER_PORT                1026
#define SERVER_BACKLOG_SIZE        2

static struct client *
server_find_client(struct server *server, int fd)
{
    struct client *client;

    list_for_each_entry_reverse(&server->clients, client, node) {
        if (client_get_fd(client) == fd) {
            return client;
        }
    }

    return NULL;
}

static struct client *
server_alloc_client(struct server *server, int fd)
{
    struct client *client;

    client = malloc(sizeof(*client));

    if (!client) {
        return NULL;
    }

    client_open(client, fd);
    list_insert_tail(&server->clients, &client->node);

    return client;
}

static void
server_close_client(struct server *server, int fd)
{
    struct client *client;

    client = server_find_client(server, fd);
    list_remove(&client->node);
    close(client->fd);
    server->nr_clients = server->nr_clients - 1;
    free(client);
}

static void
server_close(struct server *server)
{
    assert(server);

    server_cleanup(server);
    close(server->fd);
}

int
server_init(struct server *server)
{
    int error = 0;
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    if (fd == -1) {
        perror("socket creation error:");
        return -1;
    }

    server->fd = fd;
    server->nr_clients = 0;

    list_init(&server->clients);

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

void server_cleanup(struct server *server)
{
    assert(server);

    while (!list_empty(&server->clients)) {
        struct client *client = list_first_entry(&server->clients, struct client, node);

        list_remove(&client->node);
        client_close(client);
        free(client);
    }
}

static int
server_accept_client(struct server *server)
{
    int fd;
    struct client *client;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    client_addr_size = sizeof(client_addr);
    fd = accept(server->fd, (struct sockaddr *)&client_addr, &client_addr_size);

    if (fd == -1) {
        return -1;
    }

    client = server_alloc_client(server, fd);

    if (client) {
        server->nr_clients = server->nr_clients + 1;
        printf("Accept client fd %d\n",fd);
        return 0;
    } else {
        return -1;
    }
}

static struct pollfd *
server_build_fdarray(struct server *server, nfds_t *fdarray_size)
{
    nfds_t i;
    struct pollfd *fdarray;
    struct client *client;
    int nr_clients;

    assert(server);
    assert(fdarray_size);

    nr_clients = server->nr_clients;

    fdarray = malloc((nr_clients + 1) * sizeof(fdarray[0]));

    if (!fdarray) {
        return NULL;
    }

    fdarray[0].fd = server->fd;
    fdarray[0].events = POLLIN;

    i = 1;

    list_for_each_entry_reverse(&server->clients, client, node) {
        fdarray[i].fd = client_get_fd(client);
        fdarray[i].events = POLLIN;
        i++;
    }

    *fdarray_size = i;
    return fdarray;
}

static int
server_handle_revents(struct server *server)
{
    return server_accept_client(server);
}

static int
server_handle_client_revents(struct server *server, int fd)
{
    struct client *client = server_find_client(server, fd);

    return client_process(client);
}

int
server_poll(struct server *server)
{
    struct pollfd *fdarray;
    nfds_t fdarray_size;
    int nb_events;
    int error = 0;

    fdarray = server_build_fdarray(server, &fdarray_size);

    if (!fdarray) {
        return -1;
    }

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
            if (error != 0) {
                server_close(server);
                free(fdarray);
                return -1;
            }
        } else {
            server_handle_client_revents(server, pollfd->fd);
            server_close_client(server, pollfd->fd);
        }
    }

    return 0;
}