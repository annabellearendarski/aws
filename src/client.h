#ifndef CLIENT_H
#define CLIENT_H

#include "client_priv.h"

struct server;

/*
 * Client.
 */
struct client;

/*
 * Build a client.
 */
struct client * client_create(struct server *server, int fd);

/*
 * Get the file descriptor of a client.
 */
int client_get_fd(const struct client *client);

#endif /* CLIENT_H */
