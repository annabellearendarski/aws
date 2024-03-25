#include "client.h"
#include "server.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
* tcp server implementation
*/

int
main(void)
{   
    struct server tcp_ip_server;
    struct client *client;
    int ret;
    int socketd;

    server_tcp_ip_init(&tcp_ip_server);
    
    while(1){
    
        ret=server_poll(&tcp_ip_server);
        
        if (ret == -1){
            perror("Poll error:");
            exit(EXIT_FAILURE);
        }

        list_for_each_entry(&tcp_ip_server.clients, client, node) { //

            if (server_is_new_client_connection(&tcp_ip_server,client)){
                socketd=server_accept_client(&tcp_ip_server,client);
                if(socketd == -1){
                    printf("Server is full");
                    return -1;
                } 
                struct client newClient;
                client_init(&newClient,socketd);
                server_add_client(&tcp_ip_server,&newClient);

            }  
            else{

                server_manage_revent(&tcp_ip_server,client);
            }   
        }
    }
}

