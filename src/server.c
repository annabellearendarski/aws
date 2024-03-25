#include "client.h"
#include <errno.h>
#include "list.h"
#include <netinet/in.h>
#include <poll.h>
#include "server.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct pollfd  psocketds[MAX_NB_FD];
int num_tracked_socketd = 0;

void
server_tcp_ip_init(struct server *server){
    int ret;

    server->socketd = socket(AF_INET, SOCK_STREAM, 0);

    if (server->socketd == -1){
        perror("socket creation error:"); 
        exit(EXIT_FAILURE);
    }

    server->serv_addr.sin_family = AF_INET;
    server->serv_addr.sin_port = htons(PORT);
    server->serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ret = bind(server->socketd,(struct sockaddr *) &server->serv_addr,sizeof(server->serv_addr));
    
    if (ret == -1){
        perror("Bind error:");
        exit(EXIT_FAILURE);
    }

    ret=listen(server->socketd, SERVER_BACKLOG_SIZE);
    
    if (ret == -1){
        perror("Listen error:");
        exit(EXIT_FAILURE);
    }
    list_init(&server->clients);

    struct client server_client;
    client_init(&server_client,server->socketd);
    server_add_client(server,&server_client);
}

int
server_poll(struct server *server){
    int ret;

    ret = poll(psocketds, num_tracked_socketd, 1);
    return ret;
}

static void
server_remove_from_poll(int client_poll_idx){
    for (int i = client_poll_idx - 1; i < num_tracked_socketd -1; i++){  
        psocketds[i] = psocketds[i+1];
    }  
}

static void
server_remove_client(struct server *server,struct client *client){
    list_remove(&client->node);
    server_remove_from_poll(client->poll_idx);
}



void
server_manage_revent(struct server *server,struct client *client){
    int ret;
    bool is_conn_to_close;
    
    if(psocketds[client->socketd].revents == POLLIN) {
        ret = recv(psocketds[client->socketd].fd, client->read_buf, strlen(client->read_buf) + 1,0);
        is_conn_to_close= (ret == 0 || ret == -1);
        
        if (is_conn_to_close) {
            //goto close_conn;
            printf("error");
        }
        else{
            send(psocketds[client->socketd].fd, client->rsp_buf, strlen(client->rsp_buf) +1 ,0);
        }
    }

/*close_conn:
        server_remove_client(server,client);*/
    
}

int
server_accept_client(struct server *server,struct client *client){
    int ret;

    if(num_tracked_socketd == MAX_NB_FD-1){
        printf("Cannot accept more socket\n Socket sent is ignored");
        ret = -1;
    }
    else{
         ret = accept(server->socketd, (struct sockaddr *) &client->client_addr, &client->client_addr_size);
    }
    
    return ret;
}

bool
server_is_new_client_connection(struct server *server,struct client *client){
    bool client_request_condition;

    client_request_condition = (psocketds[client->poll_idx].revents == POLLIN &&  psocketds[client->poll_idx].fd == server->socketd);
   
    if (client_request_condition){
        return 1;
    }

    return 0; 
}
static void
server_add_to_poll(int socketd){
    psocketds[num_tracked_socketd].fd = socketd;
    psocketds[num_tracked_socketd].events = POLLIN;
}

void
server_add_client(struct server *server,struct client *client){
    server_add_to_poll(client->socketd);
    client_set_poll_idx(client,num_tracked_socketd);
    list_insert_head(&server->clients,&client->node);
    num_tracked_socketd = num_tracked_socketd + 1;
}





