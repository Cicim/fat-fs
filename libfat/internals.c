/**
 * Functions for Accessing the FAT and Bitmap
 * @author Claziero
 */

#include "internals.h"

// Returns the bit value at the given block index
int bitmap_get(FatFs *fs, int block_number) {
    // Get the bitmap sector (byte) and the bit index
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;

    return (fs->bitmap_ptr[byte_index] >> bit_index) & 1;
}

// Sets the bit value at the given block index
void bitmap_set(FatFs *fs, int block_number, int value) {
    // Get the bitmap sector (byte) and the bit index
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;

    // Set the bit
    if (value)
        fs->bitmap_ptr[byte_index] |= (1 << bit_index);
    else
        fs->bitmap_ptr[byte_index] &= ~(1 << bit_index);
}

// Returns the first free block in the bitmap
int bitmap_get_free_block(FatFs *fs) {
    int bitmap_size = fs->header->blocks_count / 8;

    // Loop through the bitmap
    for (int i = 0; i < bitmap_size; i++) {
        // For every byte in the bitmap check if its value is not 0xFF
        if (fs->bitmap_ptr[i] != 0xFF) {
            // Loop through the bits in the byte
            for (int j = 0; j < 8; j++) {
                // If the bit is 0, return the block index
                if (!((fs->bitmap_ptr[i] >> j) & 1))
                    return i * 8 + j;
            }
        }
    }

    // If no free block was found, return -1
    return -1;
}
