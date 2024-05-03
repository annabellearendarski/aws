#include <stdio.h>

#include "server.h"

int
main(void)
{
    struct server server;
    int error;

    error = server_init(&server);

    // TODO Turn that into a loop and check errors.
    if (error != -1) {
        server_poll(&server);
    }

    printf("end\n");

    server_cleanup(&server);
}
