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
 */
void client_init(struct client *client, int fd);

/*
 * Return true if the fd of the given client is tracked by the server (fd != -1)
 */
bool client_is_busy(struct client *client);

/*
 * Return true if the fd of the given client is not tracked by the server 
 * (fd == -1)
 */
bool client_is_available(struct client *client);

/*
 * Close the client connection
 */
void client_close(struct client *client);

/*
 * Receive the message sent by the client and answer back "ServerNotImplemented"
 */
int client_receive(struct client *client);

#endif /* CLIENT_H */