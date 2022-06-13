/**
 * File writing function
 * @author Claziero
 */

#include <stdlib.h>
#include <string.h>
#include "internals.h"

/**
 * Writes data from a buffer into file
 * Returns a FatResult or the number of written bytes
 * @author Claziero
 */
int file_write(FileHandle *file, const char *data, int size) {
    // Check if size is valid
    if (size < 0)
        return WRITE_INVALID_ARGUMENT;

    // Check if file is valid
    if (file == NULL)
        return WRITE_INVALID_ARGUMENT;

    // Switch on "mode" of file
    FatResult res;
    int written_size = 0, num_blocks, size_to_write, num_blocks_taken;
    switch (file->mode) {
        case FILE_MODE_READ:
            return WRITE_INVALID_ARGUMENT;

        case FILE_MODE_WRITE:
            // Move the pointer to the beginning of the file
            res = file_seek(file, 0, FILE_SEEK_SET);
            if (res != OK)
                return res;

            // Calculate the number of blocks to write (0 = no more blocks)
            num_blocks = (size + sizeof(FileHeader)) / file->fs->header->block_size;
            int num_blocks_to_write = num_blocks;

            // Calculate the number of blocks taken by the file (0 = 1 block only)
            num_blocks_taken = (file->fh->size + sizeof(FileHeader)) / 
                file->fs->header->block_size;

            // If the file is too big, truncate it
            if (num_blocks < num_blocks_taken) {
                int next = file->initial_block_number;

                // Go to the last useful block
                while (num_blocks--) {
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
                int block = next;
                while (block != FAT_EOF) {
                    // Set the bitmap
                    bitmap_set(file->fs, block, 0);
                    
                    // Update the FAT table
                    next = fat_get_next_block(file->fs, block);
                    fat_set_next_block(file->fs, block, FAT_EOF);

                    block = next;
                }
            }

            // Extend the file if necessary
            else if (num_blocks > num_blocks_taken) {
                // Go to the last block
                int block = file->initial_block_number;
                while (fat_get_next_block(file->fs, block) != FAT_EOF)
                    block = fat_get_next_block(file->fs, block);

                // Get the number of blocks to add
                int num_blocks_to_add = num_blocks - num_blocks_taken;
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

            // Write in the file
            while (written_size < size) {
                // If this is the first block
                if (file->current_block_number == file->initial_block_number) {
                    size_to_write = 
                        size > file->fs->header->block_size - sizeof(FileHeader) ? 
                        file->fs->header->block_size - sizeof(FileHeader) : size;

                    // Write the data
                    memcpy(file->fs->blocks_ptr 
                        + file->current_block_number * file->fs->header->block_size 
                        + sizeof(FileHeader), 
                        data + written_size, size_to_write);

                    // Update the written size
                    written_size += size_to_write;
                }

                else {
                    size_to_write = 
                        size - written_size > file->fs->header->block_size ? 
                        file->fs->header->block_size : size - written_size;

                    // Write the data
                    memcpy(file->fs->blocks_ptr + file->current_block_number * file->fs->header->block_size, 
                        data + written_size, size_to_write);

                    // Update the written size
                    written_size += size_to_write;
                }

                // Change the block
                if (fat_get_next_block(file->fs, file->current_block_number) != FAT_EOF)
                    file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);
            }

            // Update size
            file->fh->size = written_size;

            // Update the offset
            file->offset = written_size 
                - num_blocks_to_write * file->fs->header->block_size
                + sizeof(FileHeader);            

            // TODO: Update modification time

            break;

        case FILE_MODE_APPEND:
            // Move the pointer to the end of the file
            res = file_seek(file, 0, FILE_SEEK_END);
            if (res != OK)
                return res;

            // Calculate the offset of the last block
            int last_block_offset = 
                (file->fh->size + sizeof(FileHeader)) % file->fs->header->block_size;

            // Calculate the number of blocks to write (0 = no more blocks)
            num_blocks = (size + last_block_offset) / file->fs->header->block_size;

            // Calculate the number of blocks taken by the file (0 = 1 block only)
            num_blocks_taken = (file->fh->size + sizeof(FileHeader)) / 
                file->fs->header->block_size;

            // Extend the file if necessary
            if (num_blocks > 0) {
                // Go to the last block
                int block = file->initial_block_number;
                while (fat_get_next_block(file->fs, block) != FAT_EOF)
                    block = fat_get_next_block(file->fs, block);

                // Add blocks
                while (num_blocks--) {
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

            // Write in the file
            while (written_size < size) {
                // If this is the first block
                if (file->current_block_number == file->initial_block_number) {
                    size_to_write = 
                        size > file->fs->header->block_size - last_block_offset ? 
                        file->fs->header->block_size - sizeof(FileHeader) : size;

                    // Write the data
                    memcpy(file->fs->blocks_ptr 
                        + file->current_block_number * file->fs->header->block_size
                        + last_block_offset, 
                        data + written_size, size_to_write);

                    // Update the written size
                    written_size += size_to_write;
                }

                else {
                    size_to_write = 
                        size - written_size > file->fs->header->block_size ? 
                        file->fs->header->block_size : size - written_size;

                    // Write the data
                    memcpy(file->fs->blocks_ptr 
                        + file->current_block_number * file->fs->header->block_size
                        + file->offset, 
                        data + written_size, size_to_write);

                    // Update the written size
                    written_size += size_to_write;
                }

                // Change the block
                if (fat_get_next_block(file->fs, file->current_block_number) != FAT_EOF)
                    file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);
            }

            // Update size
            file->fh->size += written_size;

            // Update the offset
            file->offset = written_size 
                - num_blocks * file->fs->header->block_size
                + sizeof(FileHeader);

            // TODO: Update modification time

            break;

        default:
            return WRITE_INVALID_ARGUMENT;
    }

    return written_size;
}
