/**
 * Header File for Internal Structs and Functions
 * @author Cicim
 */
#include "fat.h"


/**
 * Bitmap
 */
// Returns the bit value at the given block index
int bitmap_get(FatFs *fs, int block_number);
// Sets the bit value at the given block index
void bitmap_set(FatFs *fs, int block_number, int value);
// Returns the first free block in the bitmap
int bitmap_get_free_block(FatFs *fs);

/**
 * FAT
 */
#define FAT_EOF -1

// Returns the next block in the FAT table
#define fat_get_next_block(fs, block_number) (fs->fat[block_number])
// Sets the next block in the FAT table
#define fat_set_next_block(fs, block_number, next_block)\
    (fs->fat[block_number] = next_block)
