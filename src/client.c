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

/*
int
client_process(struct client *client)
{
    ssize_t nr_bytes;
    char recv_buf[10];

    nr_bytes = recv(client_get_fd(client), recv_buf, sizeof(recv_buf), 0);

    printf("Process client %d",client_get_fd(client));

    if (nr_bytes <= 0){
        if (errno != EWOULDBLOCK) {
            return -1;
        }
    }
    if(strchr(recv_buf,'\0') != NULL)
    {
        send(client->fd, recv_buf, nr_bytes, 0);
    }

    return 0;
}*/

int
client_process(struct client *client)
{
    ssize_t nr_bytes_rcv;
    ssize_t nr_bytes_sent;
    char recv_buf[10];
    int offset = 0;

    while (1) {
        size_t max_len = sizeof(recv_buf) - offset;
        nr_bytes_rcv = recv(client->fd, recv_buf+offset, max_len, 0);
        if (nr_bytes_rcv == -1) {
            goto fail;
        }
        if (nr_bytes_rcv == 0) {
            printf("Close connection : client %d",client->fd);
            break;
        }
        offset += nr_bytes_rcv;
        if (offset == 10) {
            printf("message received");
            break;
        }
    }

    nr_bytes_sent = send(client->fd, recv_buf, offset, 0);

    if (nr_bytes_sent == -1) {
        goto fail;
    }

    return 0;

fail:
    perror("Error:");
    return -1;
}