#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "client.h"

#define SERVER_MAX_NR_CLIENT       10

struct server {
    struct client clients[SERVER_MAX_NR_CLIENT];
    int fd;
};

/*
 * Initialize a server.
 *
 * The server is initialized as a tcp ip server
 * 
 * If successful, return 0. If error occured during initialization -1 is returned. 
 */
int server_init(struct server *server);

/*
 * Polling a server.
 *
 * Monitor events on a set of file descriptors and interact according those 
 * events.
 * The function uses "poll" system call.
 * Only POLLIN events are handled.
 * 
 * If successful, return 0. If error occured during event handling or connection 
 * is closed -1 is returned. 
 */
int server_poll(struct server *server);

#endif /* SERVER_H */