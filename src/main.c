#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "server.h"

/*
* tcp server implementation
*/

int
main(void)
{   
    struct server tcp_ip_server;

    server_tcp_ip_init(&tcp_ip_server);
    server_poll(&tcp_ip_server);
}