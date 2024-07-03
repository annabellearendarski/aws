#include <stdlib.h>

#include "server.h"

int
main(void)
{
    struct server server;
    int error;

    error = server_init(&server);

    if (error) {
        return EXIT_FAILURE;
    }

    do {
        error = server_poll(&server);
    } while (!error);

    return EXIT_FAILURE;
}
