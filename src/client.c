#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"

void
client_init(struct client *client)
{
    assert(client);

    client->fd = -1;
}

void
client_open(struct client *client, int fd)
{
    assert(client);
    assert(fd >= 0);

    client->fd = fd;
}

void
client_close(struct client *client)
{
    assert(client);

    close(client->fd);
    client->fd = -1;
}

bool
client_is_closed(struct client *client)
{
    assert(client);

    return (client->fd == -1);
}

int
client_get_fd(const struct client *client)
{
    assert(client);

    return client->fd;
}

void
client_process(struct client *client)
{
    ssize_t nr_bytes_rcv;
    ssize_t nr_bytes_sent;
    char buffer[512];
    int error;

    nr_bytes_rcv = recv(client->fd, buffer, sizeof(buffer), 0);

    if (nr_bytes_rcv == -1) {
        error = errno;
    } else {
        if (nr_bytes_rcv == 0) {
            printf("client%d: closed\n", client->fd);
            shutdown(client->fd, SHUT_RDWR);

            error = ENOTCONN;
        } else {
            printf("%s", buffer);

            if (nr_bytes_sent == -1) {
                error = errno;
            } else {
                error = 0;
            }
        }
    }
    //TODO : how to handle errors ?
}
