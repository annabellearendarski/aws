#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "entry.h"
#include "http_request.h"
#include "http_response.h"
#include "server.h"

//1 -


/*response =>
- header string ,
- buffer(is created to avoid confusion between binary 0 and text null terminating caracter ) body
=> instancier (union ? )
buffer
request : buffer(in case /string path*/

static int
client_build_http_response(struct client *client, struct http_request *request,
                           struct http_response *response)
{
    http_response_init(response);// = http_transaction_create(request);

    struct aws_string extracted_path;
    aws_string_init_empty(&extracted_path);
    http_retrieve_requested_ressource_path(request, &extracted_path);

    unsigned int entry_kind;
    entry_kind = entry_find_type(extracted_path.buffer);
    int error = 0;

    printf("entry type %d\n", entry_kind);
    printf("path %s\n", extracted_path.buffer);


    if (entry_kind == ENTRY_DIR) {
        printf("it is a dir\n");
        struct entry_list list;

        entry_list_init(&list);

        error = entry_list_retrieve_folder_entries(&list,
                                              extracted_path.buffer);

        error = http_response_add_response_for_folder_request(response, &list, extracted_path.buffer);

        entry_list_cleanup(&list);

    } else if (entry_kind == ENTRY_FILE) {
        printf("It is file\n");
        int file_fd = open(extracted_path.buffer, O_RDONLY);
        FILE *file = fdopen(file_fd, "r");
        const char *mime_type;
        const char *file_extension;

        if (file_fd == -1) {
            error = http_response_add_response_error(response);
        } else {
            file_extension = aws_string_extract_after_last_dot(&extracted_path);
            printf("file extension %s\n", file_extension);
            mime_type = entry_retrieve_content_type(file_extension);
            off_t file_size = file_retrieve_file_size(file_fd);
            error = http_response_add_response_for_file_request(response, mime_type, file_size, file_fd);
        }

        close(file_fd);
        fclose(file);
    } else {
        printf("It is not known\n");
        error = http_response_add_response_error(response);
    }

    return error;
}

static void *
client_run(void *arg)
{
    struct client *client = arg;

    assert(client);

    for (;;) {
        char buffer[512];
        ssize_t nr_bytes_rcv;
        bool is_request_init = false;
        struct http_request http_request;
        int error = 0;

        nr_bytes_rcv = recv(client->fd, buffer, sizeof(buffer), 0);
        printf("nr bytes rcv %ld\n", nr_bytes_rcv);

        if (nr_bytes_rcv == -1) {
            perror("error : ");
            break;
        } else {

            if (nr_bytes_rcv == 0) {
                printf("client%d: closed\n", client->fd);
                break;
            } else {

                //init request
                // append request
                // should not use create , =>build
                // la logique de la reponse dans le client :
                // client shoul nt have direct access to object response " separation stricte, privé" response_add_header and not response set content
                // accumulate bytes until request is completely received
                // http_transaction is static , ( but attribut can me allocated dynamically )

                if (!is_request_init){
                    http_request_init(&http_request);
                }
                //append (renvoie)
                error = http_request_append(&http_request, buffer, nr_bytes_rcv);
                struct http_response http_response;
                if (!error) {
                    error = client_build_http_response(client, &http_request, &http_response);
                }

                if (!error) {
                    send(client->fd, http_response.header.buffer,// possibly send bytes per bytes
                             http_response.header.length, 0);
                    send(client->fd, http_response.body.buffer,// possibly send bytes per bytes
                             http_response.body.length, 0);
                }


                //TODO destroy
                http_request_destroy(&http_request);
                http_response_destroy(&http_response);
            }
        }
    }

    shutdown(client->fd, SHUT_RDWR);
    close(client->fd);
    server_remove_client(client->server, client);

    free(client);

    return NULL;
}

struct client *
client_create(struct server *server, int fd)
{
    struct client *client;
    int error;

    client = malloc(sizeof(*client));
    if (!client) {
        return NULL;
    }
    client->server = server;
    client->fd = fd;

    error = pthread_create(&client->pthread, NULL, client_run, client);

    if (error != 0) {
        free(client);
        return NULL;
    }

    pthread_detach(client->pthread);

    return client;
}

int
client_get_fd(const struct client *client)
{
    assert(client);

    return client->fd;
}
