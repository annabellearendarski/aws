#ifndef SERVER_H
#define SERVER_H

#include "list.h"
#include <sys/socket.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "client.h"

#define MAX_NB_FD           10
#define PORT                1026
#define SERVER_BACKLOG_SIZE 2


struct server {
    struct list clients;
    int socketd;
    struct sockaddr_in serv_addr;
};

void server_tcp_ip_init(struct server *server);
int server_poll(void);
bool server_is_pollin_revent(struct client *client);
void server_manage_pollin_revent(struct client *client);
int server_accept_client(struct server *server,struct client *client);
bool server_is_new_client_connection(struct server *server,struct client *client);
void server_add_client(struct server *server,struct client *client);

#endif /* SERVER_H */