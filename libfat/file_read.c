/**
 * File read function
 * @author Claziero
 */

#include "fat.h"
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
    while (curr_block != file->current_block_number)
        total_offset += file->fs->header->block_size;
    total_offset += file->offset - sizeof(FileHeader);

    // Check if size exceeds the file size from the current offset
    if (total_offset + size >= file->fh->size)
        size = file->fh->size - total_offset;

    // Check if offset + size exceeds the block size
    int read_size = 0;
    while (file->offset + size >= file->fs->header->block_size) {
        // Go to the current block of the file
        data = file->fs->blocks_ptr 
            + file->current_block_number * file->fs->header->block_size 
            + file->offset;

        // Read until the end of the block
        memcpy(buffer, data, file->fs->header->block_size - file->offset);

        // Update the size of the read and the buffer pointer
        read_size += file->fs->header->block_size - file->offset;
        size -= read_size;
        buffer += read_size;

        // Update the offset and the current block number
        file->offset = 0;
        file->current_block_number = file->fs->fat_ptr[file->current_block_number];

        // Check if the current block is the end of the file
        if (file->current_block_number == -1)
            return read_size;
    }
    if (read_size != 0)
        return read_size;
        
    // Else read for the given size
    data = file->fs->blocks_ptr
        + file->current_block_number * file->fs->header->block_size 
        + file->offset;
    memcpy(buffer, data, size);

    // Update the offset
    file->offset += size;

    return size;
}
