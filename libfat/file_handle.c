/**
 * File operations functions
 * @author Claziero
 */

#include "fat.h"
#include <stdlib.h>

// Returns a file descriptor (struct FileHandle) given a block number 
FatResult file_open_by_block(FatFs *fs, int block_number, FileHandle **file) {
    // Check if the block number is valid
    if (block_number < 0 || block_number >= fs->header->blocks_count)
        return INVALID_BLOCK;

    // Allocate memory for the file handle
    *file = malloc(sizeof(FileHandle));
    if (*file == NULL)
        return -1;
    
    // Initialize the file handle
    (*file)->fs = fs;
    (*file)->initial_block_number = block_number;
    (*file)->current_block_number = block_number;
    (*file)->offset = sizeof(FileHeader); // Offset initially pointing to the actual data
    (*file)->fh = (FileHeader *) (fs->blocks_ptr + block_number * fs->header->block_size);

    return OK;
}

// Creates a file handle given a path
// returns an error if path is invalid
FatResult file_open(FatFs *fs, const char *path, FileHandle **file) {
    // TBD

    return OK;
}

// Frees the memory occupied by a file handle
FatResult file_close(FileHandle *file) {
    free(file);
    return OK;
}
