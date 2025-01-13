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


static int
build_http_response(struct http_request *request,
                    struct http_response *response)
{
    int error = 0;
    unsigned int entry_kind;
    struct aws_string extracted_path;

    http_response_init(response);

    aws_string_init_empty(&extracted_path);
    error = http_retrieve_requested_ressource_path(request, &extracted_path);

    if (!error) {
        entry_kind = entry_find_type(extracted_path.buffer.buffer);

        printf("entry type %d\n", entry_kind);
        printf("path %s\n", extracted_path.buffer.buffer);

        if (entry_kind == ENTRY_DIR) {
            printf("it is a dir\n");
            struct entry_list list;

            entry_list_init(&list);

            error = entry_list_retrieve_folder_entries(&list,
                                                       extracted_path.buffer.buffer);

            error = http_response_add_response_for_folder_request(
                response, &list, extracted_path.buffer.buffer);

            entry_list_cleanup(&list);

        } else if (entry_kind == ENTRY_FILE) {
            printf("It is file\n");
            int file_fd = open(extracted_path.buffer.buffer, O_RDONLY);
            FILE *file = fdopen(file_fd, "r");
            const char *mime_type;
            const char *file_extension;

            if (file_fd == -1) {
                error = http_response_add_response_error(response);
            } else {
                file_extension =
                    aws_string_extract_after_last_dot(&extracted_path);
                printf("file extension %s\n", file_extension);
                mime_type = entry_retrieve_content_type(file_extension);
                off_t file_size = file_retrieve_file_size(file_fd);

                error = http_response_add_response_for_file_request(
                    response, mime_type, file_size, file_fd);
            }

            close(file_fd);
            fclose(file);
        } else {
            printf("It is not known\n");
            error = http_response_add_response_error(response);
        }

        aws_buffer_destroy(&extracted_path.buffer);
    }
    return error;
}

static void *
client_run(void *arg)
{
    struct client *client = arg;
    bool is_request_init = false;

    assert(client);

    for (;;) {
        char buffer[512];
        ssize_t nr_bytes_rcv;

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

                if (!is_request_init) {
                    http_request_init(&http_request);
                    is_request_init = true;
                }

                error =
                    http_request_append(&http_request, buffer, nr_bytes_rcv);

                if (!error) {
                    struct http_response http_response;
                    error = build_http_response(&http_request, &http_response);

                    send(client->fd, http_response.header.buffer.buffer,
                         http_response.header.buffer.length, 0);
                    send(client->fd, http_response.body.buffer,
                         http_response.body.length, 0);

                    http_request_destroy(&http_request);
                    http_response_destroy(&http_response);
                    is_request_init = false;
                    break;
                }
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
