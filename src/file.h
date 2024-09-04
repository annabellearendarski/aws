#ifndef FILE_H
#define FILE_H

#include "list.h"

struct entry
{
    unsigned int type;
    char *name;
    struct list node;

    //size
    //date
};

enum {
    ENTRY_DIR,
    ENTRY_FILE,
    ENTRY_UNK,
};

unsigned int file_find_entry_type(char *entry_path);

struct list * file_list_folder_entries(const char *dir_path, int *nr_entries);

const char * file_retrieve_signature(int file_fd);

__off_t file_retrieve_file_size(int file_fd);

#endif /* FILE_H */
