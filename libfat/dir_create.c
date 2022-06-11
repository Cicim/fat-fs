/**
 * Function for creating a directory
 * @author Cicim
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "internals.h"


/**
 * Insert an entry in a directory, and returns it
 * @author Cicim
 */
FatResult dir_insert(FatFs *fs, int block_number, DirEntry **entry, DirEntryType type, const char *name) {
    FatResult res;
    
    // Reserve space for the child block
    int child_block = bitmap_get_free_block(fs);
    if (child_block == FAT_EOF)
        return NO_FREE_BLOCKS;
    bitmap_set(fs, child_block, 1);

    // Get the directory size
    DirEntry *curr;
    DirHandle dir;
    dir.block_number = block_number;
    dir.count = 0;

    while (1) {
        res = dir_handle_next(fs, &dir, &curr);

        // Make sure the name is not already used
        if (res == OK && strcmp(curr->name, name) == 0) {
            // Free the child block
            bitmap_set(fs, child_block, 0);

            return FILE_ALREADY_EXISTS;
        }

        // If you found a DIR_END, store its address
        if (curr->type == DIR_END) {
            *entry = curr;
            dir.count++;
            break;
        }
    }

    // Extend the directory if necessary
    if (dir.count % ENTRIES_PER_BLOCK(fs) == 0) {
        // Get a new block
        int next_block = bitmap_get_free_block(fs);
        // If there are no free blocks, return an error
        if (next_block == FAT_EOF) {
            // Free the child block
            bitmap_set(fs, child_block, 0);

            return NO_FREE_BLOCKS;
        }
        bitmap_set(fs, next_block, 1);
        // Set the next block
        fat_set_next_block(fs, dir.block_number, next_block);
        // Set the block number
        dir.block_number = next_block;
    }

    // Insert an empty DirEntry at the current offset
    DirEntry *ptr = (DirEntry *)fs->blocks_ptr 
        + dir.block_number * ENTRIES_PER_BLOCK(fs) + dir.count % ENTRIES_PER_BLOCK(fs);
    memset(ptr, 0, sizeof(DirEntry));

    // Update the values for the child entry
    (*entry)->type = type;
    strcpy((*entry)->name, name);
    (*entry)->first_block = child_block;

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
    res = dir_insert(fs, parent_block, &entry, DIR_ENTRY_DIRECTORY, name);
    if (res != OK) {
        return res;
    }

    char *block_ptr = fs->blocks_ptr + entry->first_block * fs->header->block_size;
    // Fill the block with zeros
    memset(block_ptr, 0, fs->header->block_size);

    return OK;
}
