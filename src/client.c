#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
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

int 
client_process(struct client *client)
{
    ssize_t nr_bytes;
    char recv_buf[10];
    const char *send_buf = "serverNotImplemented";

    nr_bytes = recv(client->fd, recv_buf, sizeof(recv_buf), MSG_DONTWAIT);

    if (nr_bytes <= 0){
        return -1;
    }
    
    send(client->fd, send_buf, strlen(send_buf), 0);
    
    return 0;
}
