#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "errorCodes.h"
#include "file.h"
#include "list.h"

#define FILE_PDF_SIGNATURE 0x0000002D46445025
#define FILE_JPEG_SIGNATURE 0x000000FFE0FFD8FF

entry_type
file_find_entry_type(char *entry_path)
{
    int error;
    struct stat path_stat;

    error = stat(entry_path, &path_stat);

    if (error) {
        return ENTRY_UNK;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        return ENTRY_DIR;
    } else if (S_ISREG(path_stat.st_mode)) {
        return ENTRY_FILE;
    } else {
        return ENTRY_UNK;
    }
}

void
file_list_init(struct file_list *list)
{
    assert(list);
    list_init(&list->entries);
}

errorCode
file_list_retrieve_folder_entries(struct file_list *list, const char *dir_path)
{
    assert(list);

    DIR *dir_stream;
    struct dirent *next_dir_entry;
    int i = 0;
    errorCode error = 0;

    dir_stream = opendir(dir_path);

    if (dir_stream) {
        errno = 0;

        while (((next_dir_entry = readdir(dir_stream)) != NULL) && !error) {
            struct entry *entry;
            entry = malloc(sizeof(*entry));

            if (!entry) {
                error = MALLOC_FAILED;
            }

            if (!error) {
                entry->name = malloc(strlen(next_dir_entry->d_name) + 1);
                strcpy(entry->name, next_dir_entry->d_name);
                entry->type = file_find_entry_type(next_dir_entry->d_name);
                list_insert_head(&list->entries, &entry->node);

                i++;
                errno = 0;
            }
        }

        if (errno != 0) {
            error = ERROR;
        }

    } else {
        error = INVALID_PARAMETER;
    }

    closedir(dir_stream);
    return error;
}

void
file_list_cleanup(struct file_list *list)
{
    while (!list_empty(&list->entries)) {
        struct entry *entry =
                list_first_entry(&list->entries, struct entry, node);
        free(entry->name);
        list_remove(&entry->node);
    }
}

/*
 * Retrieve file signature.
 * Extract the first 8 bytes of the file and
 * compare the extracted value to different signature with the help of a mask
 * to adjust different size of signatures.
 */
const char *
file_retrieve_signature(FILE *file)
{
    const char *mime_type;
    unsigned long file_signature;

    fread(&file_signature, 1, 8, file);
    fseek(file, 0, SEEK_SET);

    if (((file_signature & 0x000000FFFFFFFFFF) ^ FILE_PDF_SIGNATURE) == 0) {
        mime_type = "application/pdf";
    } else if (((file_signature | 0xFFFFFFFF00000000) ^ FILE_JPEG_SIGNATURE) ==
               0) {
        mime_type = "image/jpeg";
    } else {
        mime_type = "";
    }

    return mime_type;
}

off_t
file_retrieve_file_size(int file_fd)
{
    struct stat file_stat;
    fstat(file_fd, &file_stat);

    return file_stat.st_size;
}
