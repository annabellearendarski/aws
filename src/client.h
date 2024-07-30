#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <pthread.h>

#include "hlist.h"

struct server;

/*
 * Client.
 */
struct client {
    int fd;
    struct hlist_node node;
    struct server *server;
    pthread_t pthread;
};

/*
 * Initialize a client.
 */
void client_init(struct client *client);

/*
 * Set server.
 */
void client_set_server(struct client *client, struct server *server);

/*
 * Open a client.
 *
 * The given file descriptor must denote a valid connected socket.
 */
void client_open(struct client *client, int fd);

/*
 * Close a client.
 *
 * The associated file descriptor is closed.
 */
void client_close(struct client *client);

/*
 * Check if a client is closed.
 */
bool client_is_closed(struct client *client);

/*
 * Get the file descriptor of a client.
 */
int client_get_fd(const struct client *client);

/*
 * Create client thread.
 *
 * The client must be open before calling this function.
 */
int client_create_thread(struct client *client);

#endif /* CLIENT_H */
