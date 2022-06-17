/**
 * File creation functions
 * @authors Cicim, Claziero
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "internals.h"

/**
 * Create a file inside a directory
 * @authors Cicim, Claziero
 */
FatResult file_create(FatFs *fs, const char *path) {
    FatResult res;

    // Transform the path into an absolute path
    char path_buffer[MAX_PATH_LENGTH];
    char *dir_path, *name;
    res = path_get_components(fs, path, path_buffer, &dir_path, &name);
    if (res != OK)
        return res;

    // Get the block number of the parent directory
    int parent_block;
    res = dir_get_first_block(fs, dir_path, &parent_block);
    if (res != OK)
        return res;

    // Get an entry in the parent directory
    DirEntry *entry;
    res = dir_insert(fs, parent_block, &entry, FAT_EOF, DIR_ENTRY_FILE, name);
    if (res != OK)
        return res;

    // Fill the file header
    FileHeader *header = (FileHeader *)(fs->blocks_ptr + entry->first_block * fs->header->block_size);
    header->size = 0;
    
    // Get the date
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Set "date_created" and "date_modified" dates
    header->date_created.sec = timeinfo->tm_sec;
    header->date_created.min = timeinfo->tm_min;
    header->date_created.hour = timeinfo->tm_hour;
    header->date_created.day = timeinfo->tm_mday;
    header->date_created.month = timeinfo->tm_mon + 1;
    header->date_created.year = timeinfo->tm_year + 1900;

    header->date_modified.sec = timeinfo->tm_sec;
    header->date_modified.min = timeinfo->tm_min;
    header->date_modified.hour = timeinfo->tm_hour;
    header->date_modified.day = timeinfo->tm_mday;
    header->date_modified.month = timeinfo->tm_mon + 1;
    header->date_modified.year = timeinfo->tm_year + 1900;

    return OK;
}
