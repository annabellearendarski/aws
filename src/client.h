#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <poll.h>

struct client {
    int fd;
    size_t index;
    char recv_buf[10];
    const char *send_buf;
};

void client_init(struct client *client, int fd, size_t index);
void client_set_fd(struct client *client, int fd);
int client_pollin_behaviour(struct client *client);

#endif /* CLIENT_H */