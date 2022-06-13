/**
 * File read function
 * @author Claziero
 */

#include "internals.h"
#include <stdlib.h>
#include <string.h>

/**
 * Reads data from file into a buffer
 * Returns a FatResult or the number of read bytes
 * @author Claziero
 */
int file_read(FileHandle *file, char *buffer, int size) {
    // Check if the file is open
    if (file == NULL)
        return -1;

    // Calculate the total offset from the file start
    int curr_block = file->initial_block_number;
    int total_offset = 0;
    char *data;

    if (file->current_block_number != file->initial_block_number) {
        do {
            total_offset += file->fs->header->block_size;
            curr_block = fat_get_next_block(file->fs, curr_block);
        }    
        while (fat_get_next_block(file->fs, curr_block) != FAT_EOF);
    }        
    total_offset += file->offset - sizeof(FileHeader);

    // Check if size exceeds the file size from the current offset
    if (total_offset + size >= file->fh->size)
        size = file->fh->size - total_offset;

    // Read the data
    int read_size = 0;
    while (size > 0) {
        // Go to the current block of the file
        data = file->fs->blocks_ptr 
            + file->current_block_number * file->fs->header->block_size 
            + file->offset;

        // Read until the end of the block
        int size_to_read = size > file->fs->header->block_size - file->offset ?
            file->fs->header->block_size - file->offset : size;
        memcpy(buffer, data, size_to_read);

        // Update the size of the read and the buffer pointer
        read_size += size_to_read;
        size -= size_to_read;
        buffer += size_to_read;

        // Check if the current block is the end of the file
        if (fat_get_next_block(file->fs, file->current_block_number) != FAT_EOF) {
            // Update the offset and the current block number
            file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);
            file->offset = 0;
        }
        else {
            // Else update the offset
            file->offset += size_to_read;
            return read_size;  
        } 
    }

    return read_size;
}
