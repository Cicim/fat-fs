/**
 * Move or copy files or directories.
 * @author Cicim
 */
#include <stdio.h>
#include <string.h>
#include "internals.h"


typedef struct MoveData {
    DirEntryType src_type;
    int src_block;
    int destination_block;
    char *destination_name;
// Move-only
    int src_dir_block;
    char *source_name;
} MoveData;

/**
 * Get the data to move or copy a path to another
 * @author Cicim
 */
FatResult path_get_data(FatFs *fs, const char *source_path, const char *dest_path, MoveData *path) {
    FatResult res;
    if (strcmp(source_path, dest_path) == 0)
        return SAME_PATH;

    /**
     * Part 1
     * Discover everything about the source file
     */
    DirEntryType src_type;
    int src_block;

    // Get the source path components
    char source_path_buffer[MAX_PATH_LENGTH];
    char *source_dir, *source_name;
    res = path_get_components(fs, source_path, source_path_buffer, &source_dir, &source_name);
    // path_get_components will not allow you to move the root directory
    if (res != OK)
        return res;


    int src_dir_block;
    // Get the block number of the source directory
    res = dir_get_first_block(fs, source_dir, &src_dir_block);
    if (res != OK)
        return res;

    // Get the type of the source file
    DirHandle dir;
    DirEntry *entry;
    // Get the block number of the source file
    res = dir_get_entry(fs, src_dir_block, source_name, &entry, &dir);
    if (res != OK)
        return res;

    src_type = entry->type;
    src_block = entry->first_block;

    /**
     * Part 2
     * Discover everything about the destination directory block
     */
    int destination_block;
    char *destination_name = source_name;

    // If the destination is the root
    if (strcmp(dest_path, "/") == 0)
        destination_block = ROOT_DIR_BLOCK;
    else {
        // Get the destination path components
        char dest_path_buffer[MAX_PATH_LENGTH];
        char *dest_dir, *dest_name;
        res = path_get_components(fs, dest_path, dest_path_buffer, &dest_dir, &dest_name);
        if (res != OK)
            return res;

        // Get the block number of the destination directory
        int dest_dir_block;
        res = dir_get_first_block(fs, dest_dir, &dest_dir_block);
        if (res != OK)
            return res;

        // Check if the destination name exists
        DirHandle dest_dir_handle;
        DirEntry *dest_entry;

        res = dir_get_entry(fs, dest_dir_block, dest_name, &dest_entry, &dest_dir_handle);
        if (res == OK) {
            // If it exists, and it is a file, return an error
            if (dest_entry->type == DIR_ENTRY_FILE)
                return FILE_ALREADY_EXISTS;
            // If it exists, and it is a directory, move to it
            else
                destination_block = dest_entry->first_block;
        }
        // If the entry does not exist, move it to the destination directory
        else if (res == FILE_NOT_FOUND) {
            destination_block = dest_dir_block;
            destination_name = dest_name;
        }
        else
            return res;
    }

    // Create the path structure
    path->src_type = src_type;
    path->src_block = src_block;
    path->src_dir_block = src_dir_block;
    path->source_name = source_name;
    path->destination_block = destination_block;
    path->destination_name = destination_name;

    return OK;
}

/**
 * Copy the file and directory structures
 * @author Cicim
 */
FatResult file_copy_recursive(FatFs *fs, int src_block, int src_type, int *copy_block) {
    FatResult res;

    // For all blocks in the source file
    int curr_src_block = src_block;
    int curr_new_block = bitmap_get_free_block(fs);
    *copy_block = curr_new_block;

    do {
        // Copy the block with memcpy
        char *src_block_data = fs->blocks_ptr + curr_src_block * fs->header->block_size;
        char *new_block_data = fs->blocks_ptr + curr_new_block * fs->header->block_size;
        memcpy(new_block_data, src_block_data, fs->header->block_size);
        bitmap_set(fs, curr_new_block, 1);

        if (curr_src_block == EOF)
            break;

        // If this bcurr_src_block == FAT_EOFlock is EOF, stop
        curr_src_block = fat_get_next_block(fs, curr_src_block);


        // Else, get a new block
        int next_new_block = bitmap_get_free_block(fs);
        // Connect the two
        fat_set_next_block(fs, curr_new_block, next_new_block);
        // Update the current block
        curr_new_block = next_new_block;
    } while (curr_src_block != EOF);


    if (src_type != DIR_ENTRY_DIRECTORY)
        return OK;

    // If the source is a directory, copy the various files inside of the directory
    // Loop over the two directories
    DirHandle src_dir_handle;
    DirHandle new_dir_handle;
    src_dir_handle.block_number = src_block;
    new_dir_handle.block_number = *copy_block;
    src_dir_handle.count = 0;
    new_dir_handle.count = 0;
    DirEntry *src_entry;
    DirEntry *new_entry;

    while (1) {
        // Get the next entry in the source directory
        dir_handle_next(fs, &new_dir_handle, &new_entry);
        res = dir_handle_next(fs, &src_dir_handle, &src_entry);
        if (res == END_OF_DIR)
            break;
        else if (res != OK)
            return res;

        // Get the entry source block and type
        int src_entry_block = src_entry->first_block;
        int src_entry_type = src_entry->type;
        // Copy it recursively
        int new_entry_block;
        res = file_copy_recursive(fs, src_entry_block, src_entry_type, &new_entry_block);
        if (res != OK)
            return res;

        // Copy the new block to this entry
        new_entry->first_block = new_entry_block;
    }

    return OK;
}


/**
 * Move a file or directory.
 * @author Cicim
 */
FatResult file_move(FatFs *fs, const char *source_path, const char *dest_path) {
    MoveData data;
    FatResult res = path_get_data(fs, source_path, dest_path, &data);
    if (res != OK)
        return res;

    // Create an entry in the root
    DirEntry *new_entry;
    // Write the entry to the root
    res = dir_insert(fs, data.destination_block, &new_entry, data.src_block, data.src_type, data.destination_name);
    if (res != OK)
        return res;

    // Delete the entry from the source directory
    return dir_delete(fs, data.src_dir_block, -1, data.source_name, NULL);
}

/**
 * Copy a file in a directory or vice-versa.
 * @author Cicim
 */
FatResult file_copy(FatFs *fs, const char *source_path, const char *dest_path) {
    FatResult res;
    
    MoveData data;
    res = path_get_data(fs, source_path, dest_path, &data);
    if (res != OK)
        return res;

    // Get the source block size
    int src_block_size, src_size;
    res = get_recursive_size(fs, data.src_block, data.src_type, &src_size, &src_block_size);
    if (res != OK)
        return res;

    // Exit if it is too big
    if (src_block_size + 1 > fs->header->free_blocks)
        return NO_FREE_BLOCKS;

    // Copy the source block to the destination block folder and give it the destination_name
    int new_block;
    res = file_copy_recursive(fs, data.src_block, data.src_type, &new_block);
    if (res != OK)
        return res;

    // Add an entry to the destination directory
    DirEntry *new_entry;
    return dir_insert(fs, data.destination_block, &new_entry, new_block, data.src_type, data.destination_name);
}
