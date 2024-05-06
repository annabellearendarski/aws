#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

#include "client.h"
#include "list.h"

#define SERVER_MAX_NR_CLIENT       10

struct server {
    struct list clients;
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
 * Clean up all resources used by a server.
 */
void server_cleanup(struct server *server);

/*
 * Polling a server.
 *
 * Exits when the server fails to accept a client
 *
 * TODO Don't loop here, and an return error code.
 */
void server_poll(struct server *server);

#endif /* SERVER_H */
