#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aws_string.h"
#include "entry.h"
#include "http.h"
#include "list.h"

static int
http_retrieve_requested_ressource_path(struct http_transaction *http_transaction)
{
    int error;
    struct aws_string extracted_path;

    aws_string_init_empty(&extracted_path);

    error = aws_string_extract_between(&http_transaction->request, " \t"," \t\n", &extracted_path);

    if (!error) {

        if (extracted_path.buffer[0] != '.') {
            error = aws_string_assign_format(&http_transaction->requested_path, ".%s", extracted_path.buffer);
        } else {
            error = aws_string_assign_format(&http_transaction->requested_path, "%s", extracted_path.buffer);
        }

        aws_string_destroy(&extracted_path);
    }

    return error;
}

static int
http_build_html_body_for_folder_request(
struct http_transaction *http_transaction)
{
    assert(&http_transaction->response_body);

    int error;
    struct entry_list list;

    entry_list_init(&list);

    error = entry_list_retrieve_folder_entries(&list,
                                              http_transaction->requested_path.buffer);

    if (!error) {
        error = aws_string_assign_format(&http_transaction->response_body,
                                 "<html>"
                                 "<head><title>Index of %s </title></head> "
                                 "<body>"
                                 "<h1>Index of %s </h1>"
                                 "<hr><pre>",
                                 http_transaction->requested_path.buffer,
                                 http_transaction->requested_path.buffer);

        if (!error) {
            struct entry *entry;

            list_for_each_entry(&list.entries, entry, node) {

                if (entry->type == ENTRY_DIR) {
                    error = aws_string_assign_format(&http_transaction->response_body,
                                             "<a href='%s/'>%s/</a><br>",
                                             entry->name, entry->name);
                } else {
                    error = aws_string_assign_format(&http_transaction->response_body,
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
        error = aws_string_assign_format(&http_transaction->response_body,
                                 "</pre><hr></body></html>");
    }

    entry_list_cleanup(&list);

    return error;
}

static int
http_build_response_error(struct http_transaction *http_transaction)
{
    assert(&http_transaction->response_header);
    assert(&http_transaction->response_body);

    int error;

    error = aws_string_assign_format(&http_transaction->response_header,
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 13\r\n"
                     "Connection: close\r\n"
                     "\r\n");

    error = aws_string_assign_format(&http_transaction->response_body, "404 Not Found");

    return error;
}

static int
http_build_response_for_folder_request(struct http_transaction *http_transaction)
{
    int error;

    error = http_build_html_body_for_folder_request(http_transaction);

    if (!error) {
        aws_string_assign_format(&http_transaction->response_header,
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Content-Length: %lu\r\n"
                         "\r\n",
                         http_transaction->response_body.length);
    } else {
        error = http_build_response_error(http_transaction);
    }

    return error;
}

static int
http_build_response_for_file_request(struct http_transaction *http_transaction)
{
    const char *mime_type;
    const char *file_extension;
    int error = 0;

    int file_fd = open(http_transaction->requested_path.buffer, O_RDONLY);
    FILE *file = fdopen(file_fd, "r");

    if (file_fd == -1) {
        error = http_build_response_error(http_transaction);
    } else {
        file_extension = aws_string_extract_sub_string_after_last_dot(&http_transaction->requested_path);
        printf("file extension %s\n",file_extension);
        mime_type = entry_retrieve_content_type(file_extension);
        off_t file_size = file_retrieve_file_size(file_fd);

        error = aws_string_assign_format(&http_transaction->response_header,
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

            error = aws_string_append_buffer(&http_transaction->response_body,response,file_size);
            free(response);
        }

        if (error) {
            error = http_build_response_error(http_transaction);
        }

        close(file_fd);
        fclose(file);
    }

    return error;
}

static int
http_build_response(struct http_transaction *http_transaction)
{
    unsigned int entry_kind;
    entry_kind = entry_find_type(http_transaction->requested_path.buffer);
    int error = 0;

    printf("entry type %d\n", entry_kind);
    printf("path %s\n", http_transaction->requested_path.buffer);

    if (entry_kind == ENTRY_DIR) {
        printf("it is a dir\n");
        error = http_build_response_for_folder_request(http_transaction);
    } else if (entry_kind == ENTRY_FILE) {
        printf("It is file\n");
        error = http_build_response_for_file_request(http_transaction);
    } else {
        printf("It is not known\n");
        error = http_build_response_error(http_transaction);
    }

    if (!error) {
        error = aws_string_append(&http_transaction->response, &http_transaction->response_header);
    }

    if (!error) {
        error = aws_string_append(&http_transaction->response, &http_transaction->response_body);
    }

    return error;
}

struct http_transaction *
http_transaction_create(char *request)
{
    struct http_transaction *http_transaction;
    int error;

    http_transaction = malloc(sizeof(*http_transaction));

    if (!http_transaction) {
        return NULL;
    }

    aws_string_init_empty(&http_transaction->request);
    error = aws_string_append_buffer(&http_transaction->request, request, strlen(request));

    if (!error) {
        aws_string_init_empty(&http_transaction->requested_path);
        error = http_retrieve_requested_ressource_path(http_transaction);

        if (error) {
            aws_string_destroy(&http_transaction->request);
        }
    }

    if (!error) {
        aws_string_init_empty(&http_transaction->response_header);
        aws_string_init_empty(&http_transaction->response_body);
        aws_string_init_empty(&http_transaction->response);

        error = http_build_response(http_transaction);
    }

    if (!error) {
        return http_transaction;
    } else {
        printf("Invalid path\n");
        free(http_transaction);
        return NULL;
    }
}

void
http_transaction_destroy(struct http_transaction *http_transaction)
{
    assert(http_transaction);

    aws_string_destroy(&http_transaction->response_header);
    aws_string_destroy(&http_transaction->response_body);
    aws_string_destroy(&http_transaction->response);
    aws_string_destroy(&http_transaction->request);
    aws_string_destroy(&http_transaction->requested_path);

    free(http_transaction);
}
