#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "awsString.h"
#include "errorCodes.h"
#include "file.h"
#include "http.h"
#include "list.h"

#define HTTP_MIN_SIZE_REQUESTED_PATH 4

/*
 * Extract ressource path from GET http request.
 * Ressource path is starting at index 4 :
 * GET /x
 * 012345
 * Ressource path is returned by adding "." as first caracter
 * which result to the following returned path : ./x
 * The minimal ressource path is the one which is pointing to the current folder
 * which will result to GET / .Thus request needs to be at least 5 bytes when
 * GET is detected in the request.
 */
static errorCode
http_retrieve_requested_ressource_path(
    struct http_transaction *http_transaction)
{
    char *end;
    int ressource_path_len;
    char *start_address_path;
    char *ressource_path;
    bool isStartingWithDot = false;
    errorCode error;

    if ((strncmp(http_transaction->request, "GET", 3) != 0) ||
        (strlen(http_transaction->request) < HTTP_MIN_SIZE_REQUESTED_PATH)) {
        return INVALID_PARAMETER;
    }

    start_address_path = http_transaction->request + 4;

    end = start_address_path;

    if (*end == '.') {
        isStartingWithDot = true;
    }

    while (*end != ' ') {
        end++;
    }

    ressource_path_len = (end - start_address_path) + 1;
    ressource_path = malloc(ressource_path_len + 1);

    if (!ressource_path) {
        return MALLOC_FAILED;
    }

    snprintf(ressource_path, ressource_path_len, "%s", start_address_path);

    if (isStartingWithDot) {
        error = awsStringAppendF(&http_transaction->requested_path,
                                                "%s",
                                                ressource_path);
    } else {
        error = awsStringAppendF(&http_transaction->requested_path,
                                                ".%s",
                                                ressource_path);
    }

    free(ressource_path);

    return error;
}

static errorCode
http_build_html_body_for_folder_request(
struct http_transaction *http_transaction)
{
    assert(&http_transaction->response_body);

    errorCode error;
    struct file_list list;

    file_list_init(&list);

    error = file_list_retrieve_folder_entries(&list,
                                              http_transaction->requested_path.pBuffer);

    if (!error) {
        error = awsStringAppendF(&http_transaction->response_body,
                                 "<html>"
                                 "<head><title>Index of %s </title></head> "
                                 "<body>"
                                 "<h1>Index of %s </h1>"
                                 "<hr><pre>",
                                 http_transaction->requested_path.pBuffer,
                                 http_transaction->requested_path.pBuffer);
        if (!error) {
            struct entry *entry;

            list_for_each_entry(&list.entries, entry, node)
            {
                if (entry->type == ENTRY_DIR) {
                    error = awsStringAppendF(&http_transaction->response_body,
                                             "<a href='%s/'>%s/</a><br>",
                                             entry->name, entry->name);
                } else {
                    error = awsStringAppendF(&http_transaction->response_body,
                                             "<a href='%s'>%s</a><br>",
                                             entry->name, entry->name);
                }
                if (error) {
                    break;
                }
            }
        }
    }

    if (!error) {
        error = awsStringAppendF(&http_transaction->response_body,
                                 "</pre><hr></body></html>");
    }

    file_list_cleanup(&list);

    return error;
}

static void
http_build_response_error(struct http_transaction *http_transaction)
{
    assert(&http_transaction->response_header);
    assert(&http_transaction->response_body);

    awsStringAppendF(&http_transaction->response_header,
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 13\r\n"
                     "Connection: close\r\n"
                     "\r\n");

    awsStringAppendF(&http_transaction->response_body, "404 Not Found");
}

static void
http_build_response_for_folder_request(
    struct http_transaction *http_transaction)
{
    errorCode error;

    error = http_build_html_body_for_folder_request(http_transaction);

    if (!error) {
        awsStringAppendF(&http_transaction->response_header,
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Content-Length: %lu\r\n"
                         "\r\n",
                         http_transaction->response_body.length);
    } else {
        http_build_response_error(http_transaction);
    }
    printf("response header %s \n", http_transaction->response_header.pBuffer);
}

static void
http_build_response_for_file_request(struct http_transaction *http_transaction)
{
    const char *mime_type;
    errorCode error;

    int file_fd = open(http_transaction->requested_path.pBuffer, O_RDONLY);
    FILE *file = fdopen(file_fd, "r");

    if (file_fd == -1) {
        http_build_response_error(http_transaction);
    } else {
        mime_type = file_retrieve_signature(file);
        off_t file_size = file_retrieve_file_size(file_fd);

        error = awsStringAppendF(&http_transaction->response_header,
                                 "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: %s\r\n"
                                 "Content-Length: %ld\r\n"
                                 "\r\n",
                                 mime_type, file_size);

        if (!error) {
            ssize_t bytes_read;
            size_t offset = 0;
            char *response = malloc(file_size + 1);

            while ((bytes_read = read(file_fd, response + offset,
                                            file_size - offset)) > 0) {
                offset += bytes_read;
            }

            error = awsStringAppendBuffer(&http_transaction->response_body,response,file_size);
            free(response);
        }

        if (error) {
            http_build_response_error(http_transaction);
        }

        close(file_fd);
        fclose(file);
    }
}

static void
http_build_response(struct http_transaction *http_transaction)
{
    unsigned int entry_kind;
    entry_kind = file_find_entry_type(http_transaction->requested_path.pBuffer);
    printf("entry type %d\n", entry_kind);
    printf("path %s\n", http_transaction->requested_path.pBuffer);

    if (entry_kind == ENTRY_DIR) {
        printf("it is a dir\n");
        http_build_response_for_folder_request(http_transaction);
    } else if (entry_kind == ENTRY_FILE) {
        printf("It is file\n");
        http_build_response_for_file_request(http_transaction);
    } else {
        printf("It is not known");
        http_build_response_error(http_transaction);
    }

    awsStringAppend(&http_transaction->response,
                    &http_transaction->response_header);
    awsStringAppend(&http_transaction->response,
                    &http_transaction->response_body);
}

struct http_transaction *
http_transaction_create(char *request)
{
    struct http_transaction *http_transaction;
    errorCode error;

    http_transaction = malloc(sizeof(*http_transaction));

    if (!http_transaction) {
        return NULL;
    }

    http_transaction->request = request;

    awsStringConstructEmpty(&http_transaction->requested_path);
    error = http_retrieve_requested_ressource_path(http_transaction);

    if (error) {
        free(http_transaction);
        return NULL;
    }

    awsStringConstructEmpty(&http_transaction->response_header);
    awsStringConstructEmpty(&http_transaction->response_body);
    awsStringConstructEmpty(&http_transaction->response);

    http_build_response(http_transaction);

    return http_transaction;
}

void
http_transaction_destroy(struct http_transaction *http_transaction)
{
    assert(http_transaction);

    awsStringDestroy(&http_transaction->response_header);
    awsStringDestroy(&http_transaction->response_body);
    awsStringDestroy(&http_transaction->response);
    awsStringDestroy(&http_transaction->requested_path);

    free(http_transaction);
}
