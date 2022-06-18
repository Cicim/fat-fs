/**
 * List directory contents
 * @author Cicim
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "internals.h"

/**
 * Returns the next entry in the directory given the handle
 * @author Cicim
 */
FatResult dir_handle_next(FatFs *fs, DirHandle *dir, DirEntry **entry) {
    // Get the offset in the current block
    int offset = dir->count % ENTRIES_PER_BLOCK(fs);

    // Get the pointer to the current block
    DirEntry *curr = (DirEntry *)fs->blocks_ptr + 
        dir->block_number * ENTRIES_PER_BLOCK(fs) + offset;

    // If the entry is a DIR_END, return NULL
    if (curr->type == DIR_END) {
        *entry = curr;
        return END_OF_DIR;
    }

    // Otherwise, return the entry
    *entry = curr;
    // Increment the count
    dir->count++;

    // If the count is in the next block, get the next block
    if (dir->count % ENTRIES_PER_BLOCK(fs) == 0) {
        // Get the next block
        int next_block = fat_get_next_block(fs, dir->block_number);
        // If the next block is FAT_EOF, throw an error
        if (next_block == FAT_EOF) {
            return DIR_END_NOT_FOUND;
        }

        // Set the block number
        dir->block_number = next_block;
    }

    return OK;
}

/**
 * Advances the directory handle to the next entry
 * and copies it to the given entry
 * @author Cicim
 */
FatResult dir_list(DirHandle *dir, DirEntry *entry) {
    DirEntry *curr;
    // Get the next entry
    FatResult res = dir_handle_next(dir->fs, dir, &curr);
    if (res != OK)
        return res;

    // Copy the current entry to the given space
    *entry = *curr;

    return OK;
}


// Returns the size in blocks of the given directory
FatResult file_size(FatFs *fs, const char *path, int *size, int *blocks) {
    FatResult res;

    char path_buffer[MAX_PATH_LENGTH];
    // Get the absolute path
    res = path_get_absolute(fs, path, path_buffer);
    if (res != OK)
        return res;

    if (IS_ROOT(path_buffer))
        return get_recursive_size(fs, ROOT_DIR_BLOCK, DIR_ENTRY_DIRECTORY, size, blocks);

    // Parse the path
    char *dir_path, *name;
    res = path_get_components(fs, path, path_buffer, &dir_path, &name);
    if (res != OK)
        return res;

    // Get the dir block
    int dir_block;
    res = dir_get_first_block(fs, dir_path, &dir_block);

    // Get the name entry
    DirEntry *entry;
    DirHandle dir;
    res = dir_get_entry(fs, dir_block, name, &entry, &dir);
    if (res != OK)
        return res;

    // Return the size of the entry
    return get_recursive_size(fs, entry->first_block, entry->type, size, blocks);
}

