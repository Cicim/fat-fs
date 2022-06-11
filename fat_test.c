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
void test_bitmap() {
    printf("******************************TESTING BITMAP FUNCTIONS******************************\n");

    FatFs *fs;
    fat_init("fat_test.dat", 32, 64);
    fat_open(&fs, "fat_test.dat");

    // Test bitmap_get on any block (e.g. 35) (should return 0)
    int block_number = 35;
    int bitmap_value = bitmap_get(fs, block_number);
    printf("TEST 1: bitmap_get(%d) \t\t\t= %d (should be 0)\n", block_number, bitmap_value);

    // Test bitmap_set on block n.35 (should set the bit to 1)
    bitmap_set(fs, block_number, 1);

    // Test bitmap_get on block n.35 (should return 1)
    printf("TEST 2: bitmap_set(%d, 1) \t\t= %d (should be 1)\n", block_number, bitmap_get(fs, block_number));
    
    // Test bitmap_get_free_block (should return the second block n.1)
    int free_block = bitmap_get_free_block(fs);
    printf("TEST 3: bitmap_get_free_block() \t= %d (should be 1)\n", free_block);

    // Occupy the second and the fourth blocks
    bitmap_set(fs, 1, 1);
    bitmap_set(fs, 3, 1);

    // Try again to get a free block (should return the third block n.2)
    free_block = bitmap_get_free_block(fs);
    printf("TEST 4: bitmap_get_free_block() \t= %d (should be 2)\n", free_block);

    fat_close(fs);
    printf("************************************************************************************\n\n");
}

/**
 * Testing path absolute function ...
 * @author Claziero
 */
void test_path_absolute() {
    printf("***************************TESTING PATH_ABSOLUTE FUNCTION***************************\n");

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
    printf("************************************************************************************\n\n");
}


/**
 * @author Cicim
 */
void recursive_print_directories(FatFs *fs, int block_number, int level) {
    DirEntry *entry;
    FatResult res;

    DirHandle dir;
    dir.block_number = block_number;
    dir.count = 0;

    while (1) {
        res = dir_handle_next(fs, &dir, &entry);
        if (res == OK) {
            int i = level;
            while (i--) printf("  ");

            if (entry->type == DIR_ENTRY_DIRECTORY) {
                printf("%s (%d)\n", entry->name, entry->first_block);

                recursive_print_directories(fs, entry->first_block, level + 1);
            } else {
                printf("%s\n", entry->name);
            }
        } else if (res == END_OF_DIR) {
            break;
        } else {
            printf("Error: %d\n", res);
            break;
        }
    }
}

/**
 * @author Cicim
 */
void test_dir_create() {
    printf("***************************TESTING DIR_CREATE FUNCTION***************************\n");

    FatFs *fs;
    FatResult res;
    fat_init("fat_test.dat", 128, 64);
    fat_open(&fs, "fat_test.dat");

    printf("Creating directories...\n");
    dir_create(fs, "dir1");
    dir_create(fs, "dir2");

    dir_create(fs, "dir1/dir3");
    dir_create(fs, "dir2/dir4");
    dir_create(fs, "dir2/dir5");

    dir_create(fs, "dir1/dir3/dir6");
    dir_create(fs, "dir1/dir3/dir7");
    dir_create(fs, "dir1/dir3/dir8");

    printf("/ (%d)\n", ROOT_DIR_BLOCK);
    recursive_print_directories(fs, ROOT_DIR_BLOCK, 1);


    printf("Duplicate name\n");
    res = dir_create(fs, "dir1");
    printf("\tFatResult = %d (FatResult == FILE_ALREADY_EXISTS)\n", res);

    printf("Trying to create a directory in a non-existent directory\n");
    res = dir_create(fs, "dir1/dir3/dir9");
    printf("\tFatResult = %d (FatResult == FILE_NOT_FOUND)\n", res);


    printf("************************************************************************************\n\n");


    fat_close(fs);
}


int main(int argc, char **argv) {
    // Test bitmap functions
    test_bitmap();

    // Test path absolute function
    test_path_absolute();

    // Test dir create function
    test_dir_create();

    return 0;
}
