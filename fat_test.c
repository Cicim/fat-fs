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
    fat_init("fat_test.dat", 32, 64);
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
    printf("****************************TESTING DIR_CREATE FUNCTION*****************************\n");

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

    file_create(fs, "/dir1/a.txt");
    file_create(fs, "/dir1/b.txt");
    file_create(fs, "/dir1/c.txt");
    file_create(fs, "/dir1/d.txt");
    file_create(fs, "/dir1/e.txt");
    file_create(fs, "/dir1/f.txt");
    file_create(fs, "/dir1/g.txt");
    file_create(fs, "/dir1/h.txt");
    file_create(fs, "/dir1/i.txt");
    file_create(fs, "/dir1/j.txt");
    file_create(fs, "/dir1/k.txt");
    file_create(fs, "/dir1/l.txt");
    file_create(fs, "/dir1/m.txt");
    file_create(fs, "/dir1/n.txt");
    file_create(fs, "/dir1/o.txt");
    file_create(fs, "/dir1/p.txt");
    file_create(fs, "/dir1/q.txt");
    file_create(fs, "/dir1/r.txt");
    file_create(fs, "/dir1/s.txt");
    file_create(fs, "/dir1/t.txt");
    file_create(fs, "/dir1/u.txt");
    file_create(fs, "/dir1/v.txt");
    file_create(fs, "/dir1/w.txt");
    file_create(fs, "/dir1/x.txt");
    file_create(fs, "/dir1/y.txt");
    file_create(fs, "/dir1/z.txt");

    printf("/ (%d)\n", ROOT_DIR_BLOCK);
    recursive_print_directories(fs, ROOT_DIR_BLOCK, 1);

    // Try opening a file
    printf("Trying to open a file...\n");
    FileHandle *file;
    res = file_open(fs, "/dir1/a.txt", &file, "r");
    printf("\tFatResult = %d\n", res);

    // Close the file
    printf("Closing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    printf("Duplicate name\n");
    res = dir_create(fs, "dir1");
    printf("\tFatResult = %d (FatResult == FILE_ALREADY_EXISTS)\n", res);

    printf("Trying to create a directory in a non-existent directory\n");
    res = dir_create(fs, "dir1/dir3/dir9");
    printf("\tFatResult = %d (FatResult == FILE_NOT_FOUND)\n", res);

    
    // Change the current directory
    printf("Changing the current directory...\n");
    res = path_get_absolute(fs, "/dir1", fs->current_directory);
    printf("\tFatResult = %d\n", res);
    printf("\tpath_get_absolute(\"/dir1\") = %s\n", fs->current_directory);

    res = file_open(fs, "a.txt", &file, "r");
    printf("\tFatResult = %d\n", res);
    printf("file->initial_block_number = %d\n", file->initial_block_number);
    printf("file->block_number = %d\n", file->current_block_number);
    file_close(file);

    printf("************************************************************************************\n\n");
}

/**
 * Testing file_open and file_read functions
 * @author Claziero
 */
void test_file_read() {
    printf("*****************************TESTING FILE_READ FUNCTION*****************************\n");

    FatFs *fs;
    FatResult res;
    fat_init("fat_test.dat", 32, 64);
    fat_open(&fs, "fat_test.dat");

    // Get a free block
    int block_number = bitmap_get_free_block(fs);
    printf("writing in block_number: %d\n", block_number);

    // Open the file from its block number
    FileHandle *file;
    res = file_open_by_block(fs, block_number, &file);
    printf("res of file_open_by_block: %d\n", res);
    
    // Write the size of the file MANUALLY
    char *data_ptr = fs->blocks_ptr + file->initial_block_number * fs->header->block_size;
    file->fh->size = 16;
    memcpy(data_ptr, &file->fh->size, sizeof(int));

    // Write some data to the file MANUALLY
    char data[13] = "Hello World!";
    data_ptr += file->block_offset;
    memcpy(data_ptr, data, 13);
    printf("Data written: \"%s\", real dimension: 13, set size: 16\n", data);

    // Read the data
    char buffer[300] = "";
    res = file_read(file, buffer, 3);
    printf("res of file_read(3): %d\n", res);
    printf("buffer: %s\n", buffer);

    // Make another read (should continue the string)
    res = file_read(file, buffer, 5);
    printf("res of file_read(5): %d\n", res);
    printf("buffer [should continue]: %s\n", buffer);
    
    // Make another read (should continue the string)
    res = file_read(file, buffer, 20);
    printf("res of file_read(20): %d\n", res);
    printf("buffer [should continue]: %s\n", buffer);

    // More tests to come...

    // Close the file
    file_close(file);

    fat_close(fs);
    printf("************************************************************************************\n\n");
}

/**
 * @author Claziero
 */
void test_dir_delete() {
    printf("**********************TESTING DIR_DELETE/FILE_ERASE FUNCTIONS***********************\n");

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

    file_create(fs, "/dir1/a.txt");
    file_create(fs, "/dir1/b.txt");
    file_create(fs, "/dir1/c.txt");
    file_create(fs, "/dir1/d.txt");
    file_create(fs, "/dir1/e.txt");
    file_create(fs, "/dir1/f.txt");
    file_create(fs, "/dir1/g.txt");

    printf("/ (%d)\n", ROOT_DIR_BLOCK);
    recursive_print_directories(fs, ROOT_DIR_BLOCK, 1);

    // Open a directory
    printf("\nTrying to open a directory...\n");
    DirHandle *dir;
    res = dir_open(fs, "dir1", &dir);
    printf("\tFatResult = %d\n", res);
    
    // Delete a file
    int dummy;
    printf("Deleting a file (a.txt)...\n");
    res = dir_delete(fs, dir->block_number, DIR_ENTRY_FILE, "a.txt", &dummy);
    printf("\tFatResult = %d\n", res);

    printf("Deleting a file (b.txt)...\n");
    res = dir_delete(fs, dir->block_number, DIR_ENTRY_FILE, "b.txt", &dummy);
    printf("\tFatResult = %d\n", res);

    printf("Deleting a file (c.txt)...\n");
    res = dir_delete(fs, dir->block_number, DIR_ENTRY_FILE, "c.txt", &dummy);
    printf("\tFatResult = %d\n", res);

    printf("Deleting a file (d.txt)...\n");
    res = file_erase(fs, "/dir1/d.txt");
    printf("\tFatResult = %d\n", res);

    printf("Deleting a file (g.txt)...\n");
    res = file_erase(fs, "/dir1/g.txt");
    printf("\tFatResult = %d\n\n", res);

    // Print directory tree
    printf("/ (%d)\n", ROOT_DIR_BLOCK);
    recursive_print_directories(fs, ROOT_DIR_BLOCK, 1);

    printf("************************************************************************************\n\n");
}

/**
 * Testing file_seek function ...
 * @author Claziero
 */
void test_file_seek() {
    printf("*****************************TESTING FILE_SEEK FUNCTION*****************************\n");

    FatFs *fs;
    FatResult res;
    fat_init("fat_test.dat", 64, 64);
    fat_open(&fs, "fat_test.dat");

    printf("Creating directories...\n");
    dir_create(fs, "dir1");
    dir_create(fs, "dir2");

    dir_create(fs, "dir1/dir3");
    dir_create(fs, "dir2/dir4");
    dir_create(fs, "dir2/dir5");

    file_create(fs, "/dir1/a.txt");
    file_create(fs, "/dir1/b.txt");

    printf("/ (%d)\n", ROOT_DIR_BLOCK);
    recursive_print_directories(fs, ROOT_DIR_BLOCK, 1);

    // Try opening a file
    printf("\nTrying to open a file (/dir1/a.txt)...\n");
    FileHandle *file;
    res = file_open(fs, "/dir1/a.txt", &file, "r");
    printf("\tFatResult = %d\n", res);

    // Write the size of the file MANUALLY
    char *data_ptr = fs->blocks_ptr + file->initial_block_number * fs->header->block_size;
    file->fh->size = 42;
    memcpy(data_ptr, &file->fh->size, sizeof(int));

    // Write some data to the file MANUALLY
    char data[42] = "This is a test for the file_seek function";
    data_ptr += file->block_offset;
    memcpy(data_ptr, data, 42);
    printf("Data written: \"%s\", \nreal dimension: 42, set size: 42\n\n", data);

    // Seek to the beginning of the file
    printf("TEST 1: Seeking to the beginning of the file with offset 5 ...\n");
    res = file_seek(file, 5, FILE_SEEK_SET);
    printf("\tFatResult = %d\n", res);

    // Read the data
    char buffer[300] = "";
    res = file_read(file, buffer, 10);
    printf("\tres of file_read(10): %d\n", res);
    printf("\tbuffer: %s\n", buffer);

    // Seek to offset 5 from the current pointer
    printf("TEST 2: Seeking to offset 5 from the current pointer ...\n");
    res = file_seek(file, 5, FILE_SEEK_CUR);
    printf("\tFatResult = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    res = file_read(file, buffer, 10);
    printf("\tres of file_read(10): %d\n", res);
    printf("\tbuffer: %s\n", buffer);

    // Seek to offset 9 from the end of the file
    printf("TEST 3: Seeking to offset 9 from the end of the file ...\n");
    res = file_seek(file, 9, FILE_SEEK_END);
    printf("\tFatResult = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    res = file_read(file, buffer, 10);
    printf("\tres of file_read(10): %d\n", res);
    printf("\tbuffer: %s\n", buffer);

    // More tests to come ...

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    fat_close(fs);
    printf("************************************************************************************\n\n");
}

/**
 * Testing file_write function ...
 * @author Claziero
 */
void test_file_write() {
    printf("*****************************TESTING FILE_WRITE FUNCTION****************************\n");

    FatFs *fs;
    FatResult res;
    fat_init("fat_test.dat", 64, 64);
    fat_open(&fs, "fat_test.dat");

    printf("Creating directories...\n");
    dir_create(fs, "dir1");
    dir_create(fs, "dir2");

    dir_create(fs, "dir1/dir3");
    dir_create(fs, "dir2/dir4");
    dir_create(fs, "dir2/dir5");

    file_create(fs, "/dir1/a.txt");
    file_create(fs, "/dir1/b.txt");

    printf("/ (%d)\n", ROOT_DIR_BLOCK);
    recursive_print_directories(fs, ROOT_DIR_BLOCK, 1);

    // Try opening a file
    printf("\nTrying to open a file (/dir1/a.txt) in \"w\" mode...\n");
    FileHandle *file;
    res = file_open(fs, "/dir1/a.txt", &file, "w");
    printf("\tFatResult = %d\n\n", res);

    // Write some data to the file
    char data[33] = "Test for the file_write function";
    printf("TEST 1: Trying to write \"%s\" to the file...\n", data);
    printf("\tThe file does not need another block to be allocated.\n");
    res = file_write(file, data, 33);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    char buffer[300] = "";
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Overwrite the data
    printf("TEST 2: Trying to overwrite the data in the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    char data2[43] = "This is a string that overwrites the data.";
    printf("\tTrying to write \"%s\" to the file...\n", data2);
    printf("\tThe file does not need another block to be allocated.\n");
    res = file_write(file, data2, 43);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Write more data to the file
    printf("TEST 3: Trying to write more data to the file...\n");
    char data3[76] = "This string will take more than a block to be stored because it's too long.";
    printf("\tTrying to write \"%s\" to the file...\n", data3);
    printf("\tThe file needs another block to be allocated.\n");
    res = file_write(file, data3, 76);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Reopen the file in append mode
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);
    printf("\nOpening file (/dir1/a.txt) in \"a\" mode...\n");
    res = file_open(fs, "/dir1/a.txt", &file, "a");
    printf("\tFatResult = %d\n\n", res);

    // Append some text to the file
    printf("TEST 4: Trying to append some text to the file...\n");
    char data4[52] = "This is a string that will be appended to the file.";
    printf("\tTrying to write \"%s\" to the file...\n", data4);
    printf("\tThe file needs another block to be allocated.\n");
    res = file_write(file, data4, 52);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    // Try opening an existing file in w+ mode
    printf("\nTrying to open an existing file (/dir1/b.txt) in \"w+\" mode...\n");
    res = file_open(fs, "/dir1/b.txt", &file, "w+");
    printf("\tFatResult = %d\n\n", res);

    // Write some data to the file
    char data5[35] = "Testing \"w+\" mode (existing file).";
    printf("TEST 5: Trying to write \"%s\" to the file...\n", data5);
    printf("\tThe file does not need another block to be allocated.\n");
    res = file_write(file, data5, 35);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    // Try writing a file that doesn't exist
    printf("\nTrying to open a file that doesn't exist (/dir2/c.txt) in \"w\" mode...\n");
    res = file_open(fs, "/dir2/c.txt", &file, "w");
    printf("\tFatResult = %d [-9 == FILE_NOT_FOUND]\n", res);

    // Try writing a file that doesn't exist
    printf("\nTrying to open a file that doesn't exist (/dir2/c.txt) in \"w+\" mode...\n");
    res = file_open(fs, "/dir2/c.txt", &file, "w+");
    printf("\tFatResult = %d\n\n", res);

    // Write some data to the file
    char data6[36] = "Testing \"w+\" mode (new file c.txt).";
    printf("TEST 6: Trying to write \"%s\" to the file...\n", data6);
    printf("\tThe file does not need another block to be allocated.\n");
    res = file_write(file, data6, 36);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    // Try opening an existing file in a+ mode
    printf("\nTrying to open an existing file (/dir1/b.txt) in \"a+\" mode...\n");
    res = file_open(fs, "/dir1/b.txt", &file, "a+");
    printf("\tFatResult = %d\n\n", res);

    // Append some text to the file
    char data7[49] = "This string will be appended to b.txt (a+ mode).";
    printf("TEST 7: Trying to append \"%s\" to the file...\n", data7);
    printf("\tThe file needs another block to be allocated.\n");
    res = file_write(file, data7, 49);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    // Try opening an inexisting file in a+ mode
    printf("\nTrying to open an unexisting file (/dir2/e.txt) in \"a+\" mode...\n");
    res = file_open(fs, "/dir2/e.txt", &file, "a+");
    printf("\tFatResult = %d\n\n", res);

    // Append some text to the file
    char data8[76] = "This string will be appended to e.txt which did not exist before (a+ mode).";
    printf("TEST 8: Trying to append \"%s\" to the file...\n", data8);
    printf("\tThe file needs another block to be allocated.\n");
    res = file_write(file, data8, 76);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    // Try opening an existing file in r mode
    printf("\nTrying to open an existing file (/dir1/b.txt) in \"r\" mode...\n");
    res = file_open(fs, "/dir1/b.txt", &file, "r");
    printf("\tFatResult = %d\n\n", res);

    // Try to write to the file
    char data9[50] = "This string will not be written to b.txt (r mode).";
    printf("TEST 9: Trying to write \"%s\" to the file...\n", data9); 
    printf("\tThe file is opened in \"r\" mode.\n");
    res = file_write(file, data9, 50);
    printf("\tres of file_write = %d [-18 == WRITE_INVALID_ARGUMENT]\n", res);

    // Close the file
    printf("\nClosing the file...\n");
    res = file_close(file);
    printf("\tFatResult = %d\n", res);

    // Final test: read again all the files
    printf("\nFinal test: reading all the files...\n");

    printf("Opening the file /dir1/a.txt...\n");
    res = file_open(fs, "/dir1/a.txt", &file, "r");
    printf("\tFatResult = %d\n", res);
    memset(buffer, 0, 300);
    res = file_read(file, buffer, 300);
    printf("\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    printf("Opening the file /dir1/b.txt...\n");
    res = file_open(fs, "/dir1/b.txt", &file, "r");
    printf("\tFatResult = %d\n", res);
    memset(buffer, 0, 300);
    res = file_read(file, buffer, 300);
    printf("\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    printf("Opening the file /dir2/c.txt...\n");
    res = file_open(fs, "/dir2/c.txt", &file, "r");
    printf("\tFatResult = %d\n", res);
    memset(buffer, 0, 300);
    res = file_read(file, buffer, 300);
    printf("\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    printf("Opening the file /dir2/e.txt...\n");
    res = file_open(fs, "/dir2/e.txt", &file, "r");
    printf("\tFatResult = %d\n", res);
    memset(buffer, 0, 300);
    res = file_read(file, buffer, 300);
    printf("\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    printf("Opening the file /dir2/h.txt in \"w+\" mode...\n");
    res = file_open(fs, "/dir2/h.txt", &file, "w+");
    printf("\tFatResult = %d\n\n", res);
    
    // Try to write to the file
    char data10[64] = "This      string      will      be      64      bytes      long";
    printf("TEST 10: Trying to write \"%s\" to the file...\n", data10); 
    res = file_write(file, data10, 64);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    printf("\nOpening the file /dir2/l.txt in \"a+\" mode...\n");
    res = file_open(fs, "/dir2/l.txt", &file, "a+");
    printf("\tFatResult = %d\n\n", res);

    // Try to write to the file
    char data11[48] = "This string is be 48 bytes long (+16 of header)";
    printf("TEST 11: Trying to write \"%s\" to the file...\n", data11);
    res = file_write(file, data11, 48);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    printf("\nOpening the file /dir2/r.txt in \"w+\" mode...\n");
    res = file_open(fs, "/dir2/r.txt", &file, "w+");
    printf("\tFatResult = %d\n\n", res);

    // Try to write to the file
    char data12[48] = "This string is be 48 bytes long (+16 of header)";
    printf("TEST 12: Trying to write \"%s\" to the file...\n", data12);
    res = file_write(file, data12, 48);
    printf("\tWritten bytes = %d\n", res);

    // Read the data
    memset(buffer, 0, 300);
    printf("\tTrying to read the data from the file...\n");
    res = file_seek(file, 0, FILE_SEEK_SET);
    res = file_read(file, buffer, 300);
    printf("\t\tres of file_read(300): %d\n", res);
    printf("\t\tbuffer: ");
    file_print(file, buffer);
    res = file_close(file);

    fat_close(fs);
    printf("************************************************************************************\n\n");
}


int main(int argc, char **argv) {
    // Test bitmap functions
    test_bitmap();

    // Test path absolute function
    test_path_absolute();

    // Test dir create function
    test_dir_create();

    // Test dir delete function
    test_dir_delete();

    // Test file read function
    test_file_read();

    // Test file seek function
    test_file_seek();

    // Test file write function
    test_file_write();

    return 0;
}
