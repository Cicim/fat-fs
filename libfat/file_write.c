/**
 * File writing function
 * @author Claziero
 */

#include <stdlib.h>
#include <string.h>
#include "internals.h"

#define NUM_BLOCKS_BY_SIZE(size) \
    (sizeof(FileHeader) + size) / file->fs->header->block_size;
#define OFFSET_BY_SIZE(size) \
    (sizeof(FileHeader) + size) % file->fs->header->block_size;
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * Function for changing file dimension
 * @author Claziero
 */
FatResult change_file_dimension(FileHandle *file, int size) {
    FatResult res;    
    int old_num_blocks = NUM_BLOCKS_BY_SIZE(file->fh->size);
    int new_num_blocks = NUM_BLOCKS_BY_SIZE(size);
    int offset = OFFSET_BY_SIZE(size);

    // If "offset" is 0 then the last block will be full
    // and we don't need to add another block
    if (new_num_blocks > 0 && offset == 0)
        new_num_blocks--;

    // If the file is too big, truncate it
    if (new_num_blocks < old_num_blocks) {
        int next = file->initial_block_number;

        // Go to the last useful block
        while (new_num_blocks--) {
            // Get the next block
            int next = fat_get_next_block(file->fs, next);
            
            // If the next block is unlinked, then we need to extend the file
            if (next == FAT_EOF) {
                // Get a new block
                int new_block = bitmap_get_free_block(file->fs);
                if (new_block == -1)
                    return NO_FREE_BLOCKS;

                // Set the new block as the next block
                bitmap_set(file->fs, new_block, 1);
                fat_set_next_block(file->fs, new_block, FAT_EOF);
                fat_set_next_block(file->fs, next, new_block);
                next = new_block;
            }
        }

        // Unlink the rest of the blocks if necessary
        res = fat_unlink(file->fs, next);
        if (res != OK)
            return res;
    }
    // Extend the file if necessary
    else if (new_num_blocks > old_num_blocks) {
        // Go to the last block
        int block = file->initial_block_number;
        while (fat_get_next_block(file->fs, block) != FAT_EOF)
            block = fat_get_next_block(file->fs, block);

        // Get the number of blocks to add
        int num_blocks_to_add = new_num_blocks - old_num_blocks;
        while (num_blocks_to_add--) {
            // Get a new block
            int new_block = bitmap_get_free_block(file->fs);
            if (new_block == -1)
                return NO_FREE_BLOCKS;

            // Set the new block as the next block
            bitmap_set(file->fs, new_block, 1);

            // Set the new block as the next block of the last block
            fat_set_next_block(file->fs, block, new_block);
            fat_set_next_block(file->fs, new_block, FAT_EOF);

            block = new_block;
        }
    }

    file->fh->size = size;
    return OK;
}

/**
 * Writes data from a buffer into file
 * Returns a FatResult or the number of written bytes
 * @author Claziero
 */
int file_write(FileHandle *file, const char *data, int size) {
    // Check if size is valid
    if (size <= 0)
        return WRITE_INVALID_ARGUMENT;

    // Check if file is valid
    if (file == NULL)
        return WRITE_INVALID_ARGUMENT;

    // Check if file is opened for writing
    if (!file->can_write)
        return WRITE_INVALID_ARGUMENT;

    FatResult res;
    res = change_file_dimension(file, file->file_offset + size);
    if (res != OK)
        return res;

    if (file->block_offset == file->fs->header->block_size) {
        file->block_offset = 0;
        file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);
    }
        
    // Write in the file
    int written_size = 0;
    while (written_size < size) {
        int size_to_write = MIN(size - written_size, file->fs->header->block_size - file->block_offset);
       
        // Write the data
        memcpy(file->fs->blocks_ptr 
            + file->current_block_number * file->fs->header->block_size
            + file->block_offset, 
            data + written_size, size_to_write);

        // Update the written size
        written_size += size_to_write;

        // Change the block
        if (fat_get_next_block(file->fs, file->current_block_number) != FAT_EOF) {
            file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);
            file->block_offset = 0;
        }
        else if (file->block_offset + size_to_write == file->fs->header->block_size)
            file->block_offset = file->fs->header->block_size;
        else 
            file->block_offset += size_to_write;
        
        file->file_offset += written_size;
    }

    // TODO: Update modification time

    return written_size;
}
