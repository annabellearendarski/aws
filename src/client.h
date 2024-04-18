#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>

#include <poll.h>

struct client {
    int fd;
};

/*
 * Initialize a client.
 *
 * The client is initially closed.
 */
void client_init(struct client *client);

/*!
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

/*!
 * Get the file descriptor of a client.
 */
int client_get_fd(const struct client *client);

/*
 * Process data received by a client.
 *
 * The client must be open before calling this function.
 */
int client_process(struct client *client);

#endif /* CLIENT_H */
