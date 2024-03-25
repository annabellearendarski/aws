#ifndef CLIENT_H
#define CLIENT_H

#include <poll.h>
#include <sys/socket.h>
#include "list.h"
#include <netinet/in.h>

struct client {
    struct sockaddr_in client_addr;
    socklen_t  client_addr_size;
    const char *rsp_buf;
    char read_buf[4];
    int socketd;
    struct list node;
    int poll_idx;
};

void client_init(struct client *client,int socketd);
void client_set_poll_idx(struct client *client, int poll_idx);
#endif /* CLIENT_H */