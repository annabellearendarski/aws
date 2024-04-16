#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "client.h"

void 
client_init(struct client *client, int fd)
{
    client->fd = fd;
}

static void
client_make_available(struct client *client)
{
    assert(client);

    client->fd = -1;
}

bool
client_is_busy(struct client *client)
{
    assert(client);

    return (client->fd != -1);
}

bool
client_is_available(struct client *client)
{
    assert(client);

    return (client->fd == -1);
}

int 
client_receive(struct client *client)
{
    int nb_bytes_recv;
    char recv_buf[10];
    const char *send_buf = "serverNotImplemented";
    
    nb_bytes_recv = recv(client->fd, recv_buf, sizeof(recv_buf), 0);

    if(nb_bytes_recv == 0){
        return -1;
    }
    
    send(client->fd, send_buf, strlen(send_buf), 0);
    
    return 0;
}

void
client_close(struct client *client)
{
    assert(client);

    client_make_available(client);
    close(client->fd);
}