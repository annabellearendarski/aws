#ifndef FILE_H
#define FILE_H

#include "list.h"

/*
 * Entry descriptor.
 */
struct entry {
    unsigned int type;
    char *name;
    struct list node;
    //size
    //date
};

/*
 * Enum which describes possible file mode.
 */
enum {
    ENTRY_DIR,
    ENTRY_FILE,
    ENTRY_UNK,
};

/*
 * Get the file mode (Dir, file, unknown).
 */
unsigned int file_find_entry_type(char *entry_path);

/*
 * Given a path to a directory, list all entries under it.
 */
struct list * file_list_folder_entries(const char *dir_path, int *nr_entries);

/*
 * Get the mime type of a file.
 * Possible returned values are "application/pdf", "image/jpeg" , "".
 */
const char * file_retrieve_signature(int file_fd);

/*
 * Get the size in bytes of a file.
 */
__off_t file_retrieve_file_size(int file_fd);

#endif /* FILE_H */
