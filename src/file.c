#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "file.h"
#include "list.h"

#define FILE_PDF_SIGNATURE 0x0000002D46445025
#define FILE_JPEG_SIGNATURE 0x000000FFE0FFD8FF

unsigned int
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

struct list *
file_list_folder_entries(const char *dir_path, int *nr_entries)
{
    DIR *dir_stream;
    struct list *entries;
    struct dirent *next_dir_entry;
    int i = 0;

    entries = malloc(sizeof(*entries));

    if (!entries) {
        return NULL;
    }

    list_init(entries);

    dir_stream = opendir(dir_path);

    if (dir_stream) {
        errno = 0;

        while ((next_dir_entry = readdir(dir_stream)) != NULL) {
            struct entry *entry;
            entry = malloc(sizeof(*entry));

            if (!entry) {
                closedir(dir_stream);
                free(entries);
                return NULL;
            }
            entry->name = malloc(strlen(next_dir_entry->d_name) + 1);
            strcpy(entry->name, next_dir_entry->d_name);
            entry->type = file_find_entry_type(next_dir_entry->d_name);
            list_insert_head(entries, &entry->node);

            i++;
            errno = 0;
        }

        if (errno != 0) {
            *nr_entries = 0;
            free(entries);
            return NULL;
        }

    } else {
        *nr_entries = 0;
        return NULL;
    }

    closedir(dir_stream);
    *nr_entries = i;

    return entries;
}

/*
 * Retrieve file signature.
 * Extract the first 8 bytes of the file and
 * compare the extracted value to different signature with the help of a mask
 * to adjust different size of signatures.
 */
const char *
file_retrieve_signature(int file_fd)
{
    FILE *file = fdopen(file_fd, "r");
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

__off_t
file_retrieve_file_size(int file_fd)
{
 struct stat file_stat;
    fstat(file_fd, &file_stat);

    return file_stat.st_size;
}
