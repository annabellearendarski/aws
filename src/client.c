#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "http.h"
#include "server.h"

static struct http_transaction *
client_retrieve_http_response(struct client *client, char *request)
{
    struct http_transaction *http_response = http_transaction_create(request);

    return http_response;
}

static void *
client_run(void *arg)
{
    struct client *client = arg;

    for (;;) {
        char buffer[512];
        ssize_t nr_bytes_rcv;

        nr_bytes_rcv = recv(client->fd, buffer, sizeof(buffer), 0);

        if (nr_bytes_rcv == -1) {
            perror("error : ");
            break;
        } else {

            if (nr_bytes_rcv == 0) {
                printf("client%d: closed\n", client->fd);
                break;
            } else {
                struct http_transaction *http_transaction =
                    client_retrieve_http_response(client, buffer);

                if (http_transaction) {

                    if (http_transaction->response) {
                        send(client->fd, http_transaction->response,
                             http_transaction->response_len, 0);
                        http_transaction_destroy(http_transaction);
                    }
                }
            }
        }
    }

    shutdown(client->fd, SHUT_RDWR);
    close(client->fd);
    server_remove_client(client->server, client);

    free(client);

    return NULL;
}

struct client *
client_create(struct server *server, int fd)
{
    struct client *client;
    int error;

    client = malloc(sizeof(*client));
    if (!client) {
        return NULL;
    }
    client->server = server;
    client->fd = fd;

    error = pthread_create(&client->pthread, NULL, client_run, client);

    if (error != 0) {
        free(client);
        return NULL;
    }

    pthread_detach(client->pthread);

    return client;
}

int
client_get_fd(const struct client *client)
{
    assert(client);

    return client->fd;
}
