#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

#include "client.h"
#include "hlist.h"

#define SERVER_HTABLE_SIZE  10

/*
 * Server descriptor.
 */
struct server {
    struct hlist clients[SERVER_HTABLE_SIZE];
    int fd;
    int nr_clients;
};

/*
 * Initialize a server.
 *
 * The server is initialized as a tcp ip server.
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
 * Exits when the server fails to accept a client.
 */
int server_poll(struct server *server);

/*
 * Removing a client.
 *
 * Clean up all resources used by a client.
 */
void server_remove_client(struct server *server, struct client *client);

#endif /* SERVER_H */
