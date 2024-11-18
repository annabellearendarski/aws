#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "hlist.h"
#include "macro.h"
#include "server.h"

#define SERVER_PORT 1026
#define SERVER_BACKLOG_SIZE 2

void *
utils_memcpy(const void *src, void *dest, size_t n);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_inc = PTHREAD_MUTEX_INITIALIZER;

static struct hlist *
server_get_client_bucket(struct server *server, const int fd)
{
    size_t index;

    index = fd % SERVER_HTABLE_SIZE;
    assert(index < ARRAY_SIZE(server->clients));
    return &server->clients[index];
}

static struct client *
server_alloc_client(struct server *server, int fd)
{
    struct client *client;
    struct hlist *client_bucket;

    client = client_create(server, fd);
    if (!client) {
        return NULL;
    }

    client_bucket = server_get_client_bucket(server, fd);

    if (!client_bucket) {
        free(client);
        return NULL;
    }

    pthread_mutex_lock(&mutex);

    hlist_insert_head(client_bucket, &client->node);
    server->nr_clients++;

    pthread_mutex_unlock(&mutex);

    return client;
}

void
server_remove_client(struct server *server, struct client *client)
{
    pthread_mutex_lock(&mutex);

    assert(server->nr_clients > 0);

    hlist_remove(&client->node);
    server->nr_clients--;

    pthread_mutex_unlock(&mutex);
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
    result =
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

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

    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct hlist *client_bucket = &(server->clients[i]);

        hlist_init(client_bucket);
    }

    return 0;

error:
    close(server->fd);
error_socket:
    return error;
}

void
server_cleanup(struct server *server)
{
    assert(server);

    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        struct hlist *client_bucket = &(server->clients[i]);

        while (!hlist_empty(client_bucket)) {
            struct client *client =
                hlist_first_entry(client_bucket, struct client, node);

            server_remove_client(server, client);
        }
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

    if (!client) {
        printf("Fail to accept client fd %d\n", fd);
        return -1;
    }

    printf("Accept client fd %d\n", fd);

    return 0;
}

int
server_poll(struct server *server)
{
    int error = 0;
    printf("Nr client %d\n", server->nr_clients);

    for (int i = 0; i < 10; i++) {
        error = server_accept_client(server);
    }
    server_close(server);
    goto out;
    /*if (error != 0) {
        server_close(server);
        goto out;
    }*/

out:
    return error;
}
