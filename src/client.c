#include "client.h"
#include <stddef.h>


void 
client_init(struct client *client,int socketd){
    client->rsp_buf = "ServerNotImplemented";
    client->client_addr_size = sizeof(client->client_addr);
    client->socketd = socketd;
}


void
client_set_poll_idx(struct client *client, int poll_idx){
    client->poll_idx = poll_idx;
}

