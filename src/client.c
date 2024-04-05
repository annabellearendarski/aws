#include <stddef.h>

#include "client.h"

void 
client_init(struct client *client, int fd, size_t index)
{
    client->fd = fd;
    client->index = index;
    client->send_buf = "ServerNotImplemented";
}

void
client_set_fd(struct client *client, int fd)
{
    client->fd = fd;
}

int
client_pollin_behaviour(struct client *client)
{
    int nb_bytes_recv;
    
    nb_bytes_recv = recv(client->fd, client->recv_buf, sizeof(client->recv_buf), 0);
             
    if (nb_bytes_recv == -1) {
        return -1;
    }
    else if (nb_bytes_recv == 0) {
        return -1;
    }
    else{
        send(client->fd, client->send_buf, strlen(client->send_buf), 0);
    }

    return 0;
}