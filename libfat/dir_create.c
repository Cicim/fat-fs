/**
 * Function for creating a directory
 * @author Cicim
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "internals.h"

/**
 * Returns the first block of the directory given the absolute path
 * @author Cicim
 */
FatResult dir_get_first_block(FatFs *fs, char *path, int *block_number) {
    int block = *block_number;

    if (path[0] == '/') {
        path++;
        block = ROOT_DIR_BLOCK;
    }

    if (path[0] == '\0') {
        *block_number = block;
        return OK;
    }

    // Get the name of the directory to look for
    FatResult res;
    char *name;
    DirHandle dir;
    DirEntry *entry;

    while ((name = strsep(&path, "/"))) {
        dir.block_number = block;
        dir.count = 0;

        // Get the entry with the given name
        while (1) {
            res = dir_handle_next(fs, &dir, &entry);

            if (res == END_OF_DIR) {
                return FILE_NOT_FOUND;
            }
            else if (res == OK) {
                // If you found an entry with the given name
                if (strcmp(entry->name, name) == 0) {
                    // Make sure it's a directory
                    if (entry->type != DIR_ENTRY_DIRECTORY) {
                        return NOT_A_DIRECTORY;
                    }
                    // Otherwise, return the block number
                    block = entry->first_block;
                    break;
                }
            }
            else {
                return res;
            }
        }
    }

    *block_number = block;

    return OK;
}


/**
 * Get a new entry in a directory
 * (if there is already an entry with the given name, it will fail)
 * @author Cicim
 */
FatResult dir_get_new_entry(FatFs *fs, int block_number, DirEntry **entry, const char *name) {
    FatResult res;
    DirEntry *curr;

    // Get the size of the directory
    DirHandle dir;
    dir.block_number = block_number;
    dir.count = 0;

    while (1) {
        res = dir_handle_next(fs, &dir, &curr);
        if (res == OK) {
            // If you found an entry with the same name, return an error
            if (name && strcmp(curr->name, name) == 0) {
                return FILE_ALREADY_EXISTS;
            }
            continue;
        }

        // If you found a DIR_END, return it
        if (curr->type == DIR_END) {
            *entry = curr;
            dir.count++;
            break;
        }
    }

    // Extend if necessary
    if (dir.count % ENTRIES_PER_BLOCK(fs) == 0) {
        // Get a new block
        int next_block = bitmap_get_free_block(fs);
        // If there are no free blocks, return an error
        if (next_block == FAT_EOF) {
            return NO_FREE_BLOCKS;
        }

        bitmap_set(fs, next_block, 1);
        // Set the next block
        fat_set_next_block(fs, dir.block_number, next_block);
        // Set the block number
        dir.block_number = next_block;
    }

    // Empty the first entry in dir.block_number
    DirEntry *ptr = (DirEntry *)fs->blocks_ptr + dir.block_number * ENTRIES_PER_BLOCK(fs);
    int offset = dir.count % ENTRIES_PER_BLOCK(fs);
    memset(ptr + offset, 0, sizeof(DirEntry));

    return OK;
}


/**
 * Creates a directory
 * @author Cicim
 */
FatResult dir_create(FatFs *fs, const char *path) {
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
    res = dir_get_new_entry(fs, parent_block, &entry, name);
    if (res != OK) {
        return res;
    }

    // Get the block number of the directory to create
    int child_block = bitmap_get_free_block(fs);
    if (child_block == -1) {
        return NO_FREE_BLOCKS;
    }
    bitmap_set(fs, child_block, 1);
    // Fill the block with zeros
    memset(fs->blocks_ptr + child_block * fs->header->block_size,
           0, fs->header->block_size);

    // Set the name of the entry
    strcpy(entry->name, name);
    // Set the type of the entry
    entry->type = DIR_ENTRY_DIRECTORY;
    // Set the first block of the entry
    entry->first_block = child_block;

    return OK;
}
