#include <errno.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "home.h"
#include "../init/init.h"

// Get MCPI Home Directory
char *home_get() {
    static char *dir = NULL;
    // Load
    if (dir == NULL) {
        safe_asprintf(&dir, "%s" HOME_SUBDIRECTORY_FOR_GAME_DATA, getenv("HOME"));
    }
    // Return
    return dir;
}
// Free
__attribute__((destructor)) static void _free_home() {
    free(home_get());
}

// Init
void init_home() {
    // Store Data In ~/.minecraft-pi Instead Of ~/.minecraft
    patch_address((void *) full_data_path, (void *) home_get());

    // Change Directory To Binary Directory
    char *binary_directory = get_mcpi_directory();
    if (chdir(binary_directory) != 0) {
        ERR("Unable To Change Directory: %s", strerror(errno));
    }
}
