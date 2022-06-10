/**
 * FAT File System Tester
 * @authors Cicim, Claziero
 */

#include <stdio.h>
#include <string.h>
#include "libfat/internals.h"

/**
 * Testing bitmap functions ...
 * @author Claziero
 */
void test_bitmap () {
    FatFs *fs;
    fat_init("fat_test.dat", 32, 64);
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

/**
 * Testing path absolute function ...
 * @author Claziero
 */
void test_path_absolute () {
    FatFs *fs;
    fat_init("fat_test.dat", 16, 64);
    fat_open(&fs, "fat_test.dat");

    // Change initial path to "/linus/torvalds"
    strcpy(fs->current_directory, "/linus/torvalds");

    printf("TEST 1: adding relative path \"ubuntu\" to the current path %s\n", fs->current_directory);
    FatResult res = path_get_absolute(fs, "ubuntu", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"ubuntu\") = %s\n", fs->current_directory);

    printf("TEST 2: adding absolute path \"/linus/torvalds/ubuntu/20.04/distro\"\n");
    res = path_get_absolute(fs, "/linus/torvalds/ubuntu/20.04/distro", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"/linus/torvalds/ubuntu/20.04/distro\") = %s\n", fs->current_directory);

    printf("TEST 3: going back to the parent directory using \"../\"\n");
    res = path_get_absolute(fs, "../", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"../\") = %s\n", fs->current_directory);

    printf("TEST 4: going back to the parent directory using \"../../\"\n");
    res = path_get_absolute(fs, "../../", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"../../\") = %s\n", fs->current_directory);

    printf("TEST 5: going back to the parent directory using \"../other_distro\"\n");
    res = path_get_absolute(fs, "../other_distro", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"../other_distro\") = %s\n", fs->current_directory);

    printf("TEST 6: going to the current directory using \"./\"\n");
    res = path_get_absolute(fs, "./", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"./\") = %s\n", fs->current_directory);

    printf("TEST 7: going to a directory using \"./././debian\"\n");
    res = path_get_absolute(fs, "./././debian", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"./././debian\") = %s\n", fs->current_directory);

    printf("TEST 8: going to the root directory using \"/\"\n");
    res = path_get_absolute(fs, "/", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"/\") = %s\n", fs->current_directory);

    printf("TEST 9: going to a directory using \"/linus/torvalds\"\n");
    res = path_get_absolute(fs, "/linus/torvalds", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"/linus/torvalds\") = %s\n", fs->current_directory);

    printf("TEST 10: going to a directory using \"/linus/other_distro\"\n");
    res = path_get_absolute(fs, "/linus/other_distro", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"/linus/other_distro\") = %s\n", fs->current_directory);

    printf("TEST 11: going back to the parent directory using \"..\"\n");
    res = path_get_absolute(fs, "..", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"..\") = %s\n", fs->current_directory);

    printf("TEST 12: going to the current directory using \".\"\n");
    res = path_get_absolute(fs, ".", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\".\") = %s\n", fs->current_directory);

    printf("TEST 13: going to a directory using \"...\" (error)\n");
    res = path_get_absolute(fs, "...", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"...\") = %s\n", fs->current_directory);

    printf("TEST 14: going to a directory using \"..a\" (error)\n");
    res = path_get_absolute(fs, "..a", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"..a\") = %s\n", fs->current_directory);

    fat_close(fs);
}

int main(int argc, char **argv) {
    // Test bitmap functions
    test_bitmap();

    // Test path absolute function
    test_path_absolute();

    return 0;
}
