#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "client.h"

#define SERVER_MAX_NB_FD           10
#define SERVER_PORT                1026
#define SERVER_BACKLOG_SIZE 2


struct server {
    struct client clients[50];
    int fd;
    struct pollfd pollfds[SERVER_MAX_NB_FD];
    
};

void server_tcp_ip_init(struct server *server);
void server_poll(struct server *server);

#endif /* SERVER_H */