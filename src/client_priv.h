#ifndef CLIENT_PRIV_H
#define CLIENT_PRIV_H

#include <pthread.h>

#include "hlist.h"

struct server;

struct client {
    int fd;
    struct hlist_node node;
    struct server *server;
    pthread_t pthread;
};

#endif /* CLIENT_PRIV_H */
