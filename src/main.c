#include <stdio.h>
#include <string.h>
#include "server.h"

int
main(void)
{
    struct server server;
    int error;

    error = server_init(&server);

    do {
        error = server_poll(&server);
    } while (!error);

}