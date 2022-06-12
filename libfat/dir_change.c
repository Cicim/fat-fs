/**
 * Change directory to the given path
 * @author Cicim
 */
#include <string.h>
#include "internals.h"


FatResult dir_change(FatFs *fs, const char *path) {
    FatResult res;

    // Copy the current path in a buffer
    char path_buffer[MAX_PATH_LENGTH];

    // Join the current path and the given path
    res = path_get_absolute(fs, path, path_buffer);
    if (res != OK)
        return res;

    // Make sure it's a directory
    res = dir_get_first_block(fs, path_buffer, NULL);
    if (res != OK)
        return res;
    
    // Copy the buffer to the current path
    strcpy(fs->current_directory, path_buffer);

    return OK;
}
