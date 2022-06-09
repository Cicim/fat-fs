/**
 * FAT File System Manager
 * @authors Cicim, Claziero
 */

#include <stdio.h>
#include "libfat/fat.h"

int main(int argc, char **argv) {
    FatResult ret;

    // Initialize a FAT file system
    ret = fat_init("fat.dat", 16, 128);
    if (ret != OK) {
        printf("Could not create the FAT FS\nError ID: %d\n", ret);
        return 1;
    }

    // Open the FAT file system
    FatFs *fs;
    ret = fat_open(&fs, "fat.dat");
    if (ret != OK) {
        printf("Could not open the FAT FS\nError ID: %d\n", ret);
        return 1;
    }

    // Close the FAT file system
    ret = fat_close(fs);
    if (ret != OK) {
        printf("Could not close the FAT FS\nError ID: %d\n", ret);
        return 1;
    }

    return 0;
}
