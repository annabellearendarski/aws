#include "server.h"

int
main(void)
{
    struct server tcp_ip_server;
    int error;

    error = server_init(&tcp_ip_server);

    if (error != -1) {
        server_poll(&tcp_ip_server);
    }
}
