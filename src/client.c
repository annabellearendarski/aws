#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "server.h"

static char *
client_extract_requested_folder_path(char buffer[], int buffer_size)
{
    char *string_to_parse;
    char *end;
    char *path;
    int path_len;

    string_to_parse = malloc (buffer_size);
    strncpy(string_to_parse, buffer, buffer_size);

    if ((strncmp(string_to_parse,"GET",3)!=0) || (strlen(string_to_parse) < 5)) {
        free(string_to_parse);
        return NULL;
    }

    end = string_to_parse + 4;

    while( *end != ' ') {
        end++;
    }

    path_len =  end - (string_to_parse+4);
    path = malloc(path_len + 1);

    if (!path) {
        free(string_to_parse);
        return NULL;
    }

    strncpy(path, string_to_parse + 4, path_len);
    free(string_to_parse);

    return path;
}

static void *
client_run(void *arg)
{
    struct client *client = arg;
    char *path = NULL;

    for (;;) {
        char buffer[512];
        ssize_t nr_bytes_rcv;
        char *buffer_sent =
        "HTTP/1.1 200 OK\n" "Content-Length: 12\n"
        "Content-Type: text/html\n" "Connection: close\n" "\n"
        "Hello World!";

        nr_bytes_rcv = recv(client->fd, buffer, sizeof(buffer), 0);

        if (nr_bytes_rcv == -1) {
            perror("error : ");
            break;
        } else {
            if (nr_bytes_rcv == 0) {
                printf("client%d: closed\n", client->fd);
                break;
            } else {
                ssize_t nr_bytes_sent;
                path = client_extract_requested_folder_path(buffer, 512);

                if (path) {
                    printf("path %s \n", path);
                }

                nr_bytes_sent = send(client->fd, buffer_sent, strlen(buffer_sent), 0);

                if (nr_bytes_sent == -1) {
                    perror("error : ");
                }
            }
        }
    }

    if (path) {
        free(path);
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

    return client;
}

int
client_get_fd(const struct client *client)
{
    assert(client);

    return client->fd;
}
