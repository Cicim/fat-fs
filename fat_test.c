/**
 * FAT File System Tester
 * @authors Cicim, Claziero
 */

#include <stdio.h>
#include "libfat/internals.h"

/**
 * Testing bitmap functions ...
 * @author Claziero
 */
void test_bitmap () {
    FatFs *fs;
    fat_init("fat_test.dat", 16, 64);
    fat_open(&fs, "fat_test.dat");

    // Test bitmap_get on any block (e.g. 35) (should return 0)
    int block_number = 35;
    int bitmap_value = bitmap_get(fs, block_number);
    printf("bitmap_get(%d) = %d (should be 0)\n", block_number, bitmap_value);

    // Test bitmap_set on block n.35 (should set the bit to 1)
    bitmap_set(fs, block_number, 1);

    // Test bitmap_get on block n.35 (should return 1)
    printf("bitmap_set(%d, 1) = %d (should be 1)\n", block_number, bitmap_get(fs, block_number));
    
    // Test bitmap_get_free_block (should return the first block n.0)
    int free_block = bitmap_get_free_block(fs);
    printf("bitmap_get_free_block() = %d (should be 0)\n", free_block);

    // Occupy the first and the third blocks
    bitmap_set(fs, 0, 1);
    bitmap_set(fs, 2, 1);

    // Try again to get a free block (should return the second block n.1)
    free_block = bitmap_get_free_block(fs);
    printf("bitmap_get_free_block() = %d (should be 1)\n", free_block);

    fat_close(fs);
}

int main(int argc, char **argv) {
    // Test bitmap functions
    test_bitmap();

    return 0;
}
