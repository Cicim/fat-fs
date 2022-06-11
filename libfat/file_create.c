/**
 * File creation functions
 * @author Cicim
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "internals.h"


/**
 * Create a file inside a directory
 * @author Cicim
 */
FatResult file_create(FatFs *fs, const char *path) {
    FatResult res;


    // Transform the path into an absolute path
    char abs_path_buffer[MAX_PATH_LENGTH];
    char *abs_path = abs_path_buffer;
    res = path_get_absolute(fs, path, abs_path);
    if (res != OK) {
        return res;
    }

    // Get the name of the directory to create
    char *name = strrchr(abs_path, '/') + 1;
    *(name - 1) = '\0';

    // For directories in the root, the abs_path is empty
    // so we need to set it to the root directory
    if (*abs_path == '\0') 
        abs_path = "/";

    // Get the block number of the parent directory
    int parent_block;
    res = dir_get_first_block(fs, abs_path, &parent_block);
    if (res != OK) {
        return res;
    }

    // Get an entry in the parent directory
    DirEntry *entry;
    res = dir_insert(fs, parent_block, &entry, DIR_ENTRY_FILE, name);
    if (res != OK) {
        return res;
    }

    // Fill the file header
    FileHeader *header = (FileHeader *)(fs->blocks_ptr + entry->first_block * fs->header->block_size);
    header->size = 0;
    // TODO Use the correct dates
    header->date_created = 0xCCCCCCCC;
    header->date_modified = 0x77777777;

    return OK;
}