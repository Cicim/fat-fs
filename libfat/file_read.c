/**
 * File read function
 * @author Claziero
 */

#include <stdlib.h>
#include <string.h>
#include "internals.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * Reads data from file into a buffer
 * Returns a FatResult or the number of read bytes
 * @author Claziero
 */
int file_read(FileHandle *file, char *buffer, int size) {
    // Check if the file is open
    if (file == NULL)
        return -1;

    // Check if size exceeds the file size from the current offset
    if (file->file_offset + size >= file->fh->size)
        size = file->fh->size - file->file_offset;

    // Read the data
    int read_size = 0;
    while (size > 0) {
        // Go to the current block of the file
        char *data = file->fs->blocks_ptr 
            + file->current_block_number * file->fs->header->block_size 
            + file->block_offset;

        // Read until the end of the block
        int size_to_read = MIN(size, file->fs->header->block_size - file->block_offset);
        memcpy(buffer, data, size_to_read);

        // Update the size of the read and the buffer pointer
        read_size += size_to_read;
        size -= size_to_read;
        buffer += size_to_read;
        file->file_offset += size_to_read;

        // Check if the current block is the end of the file
        if (size > 0 && fat_get_next_block(file->fs, file->current_block_number) != FAT_EOF) {
            // Update the offset and the current block number
            file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);
            file->block_offset = 0;
        }
        else {
            // Else update the offset
            file->block_offset += size_to_read;
            return read_size;  
        } 
    }

    return read_size;
}
