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
server_free_client(struct server *server, struct client *client)
{
    list_remove(&client->node);
    server->nr_clients--;

    client_close(client);
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
    struct sockaddr_in addr;
    int reuseaddr;
    int result;
    int error;
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        error = errno;
        perror("unable to create server socket");
        goto error_socket;
    }

    reuseaddr = 1;
    result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    if (result == -1) {
        error = errno;
        perror("unable to set SO_REUSEADDR");
        goto error;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    result = bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    if (result == -1) {
        error = errno;
        perror("unable to bind server socket");
        goto error;
    }

    result = listen(fd, SERVER_BACKLOG_SIZE);

    if (result == -1) {
        error = errno;
        perror("unable to listen on server socket");
        goto error;
    }

    server->fd = fd;
    server->nr_clients = 0;

    list_init(&server->clients);

    return 0;

error:
    close(server->fd);
error_socket:
    return error;
}

void server_cleanup(struct server *server)
{
    assert(server);

    while (!list_empty(&server->clients)) {
        struct client *client = list_first_entry(&server->clients, struct client, node);

        server_free_client(server, client);
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

static void
server_handle_client(struct server *server, int fd)
{
    struct client *client = server_find_client(server, fd);
    int error;

    error = client_process(client);

    if (error) {
        server_free_client(server, client);
    }
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
            error = server_accept_client(server);

            if (error != 0) {
                server_close(server);
                goto out;
            }
        } else {
            server_handle_client(server, pollfd->fd);
        }
    }

out:
    free(fdarray);
    return error;
}
