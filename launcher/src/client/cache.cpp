#include <cstdlib>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <sys/stat.h>

#include <libreborn/libreborn.h>

#include "launcher.h"
#include "cache.h"

// Get Cache Path
static std::string get_cache_path() {
    const char *home = getenv("HOME");
    if (home == NULL) {
        IMPOSSIBLE();
    }
    return std::string(home) + "/.minecraft-pi/.launcher-cache";
}

// Load
launcher_cache empty_cache = {
    .username = DEFAULT_USERNAME,
    .render_distance = DEFAULT_RENDER_DISTANCE,
    .feature_flags = {}
};
launcher_cache load_cache() {
    // Log
    DEBUG("Loading Launcher Cache...");

    // Open File
    std::ifstream stream(get_cache_path(), std::ios::in | std::ios::binary);
    if (!stream) {
        // No Warning If File Doesn't Exist
        struct stat s;
        if (stat(get_cache_path().c_str(), &s) == 0) {
            WARN("Unable To Open Launcher Cache For Loading");
        }
        return empty_cache;
    }

    // Check Version
    unsigned char cache_version;
    stream.read((char *) &cache_version, 1);
    if (stream.eof() || cache_version != (unsigned char) CACHE_VERSION) {
        WARN("Invalid Launcher Cache Version");
        stream.close();
        return empty_cache;
    }

    // Load Username And Render Distance
    launcher_cache cache;
    std::getline(stream, cache.username, '\0');
    std::getline(stream, cache.render_distance, '\0');

    // Load Feature Flags
    std::string flag;
    while (!stream.eof() && std::getline(stream, flag, '\0')) {
        if (flag.length() > 0) {
            unsigned char is_enabled = 0;
            stream.read((char *) &is_enabled, 1);
            cache.feature_flags[flag] = is_enabled != (unsigned char) 0;
        }
        stream.peek();
    }

    // Finish
    stream.close();
    if (!stream) {
        WARN("Failure While Loading Launcher Cache: %s", strerror(errno));
        return empty_cache;
    }

    // Return
    return cache;
}

// Save
#define write_env_to_stream(stream, env) \
    { \
        const char *env_value = getenv(env); \
        if (env == NULL) { \
            IMPOSSIBLE(); \
        } \
        stream.write(env_value, strlen(env_value) + 1); \
    }
void save_cache() {
    // Log
    DEBUG("Saving Launcher Cache...");

    // Open File
    std::ofstream stream(get_cache_path(), std::ios::out | std::ios::binary);
    if (!stream) {
        WARN("Unable To Open Launcher Cache For Saving");
        return;
    }

    // Save Cache Version
    unsigned char cache_version = (unsigned char) CACHE_VERSION;
    stream.write((const char *) &cache_version, 1);

    // Save Username And Render Distance
    write_env_to_stream(stream, "MCPI_USERNAME");
    write_env_to_stream(stream, "MCPI_RENDER_DISTANCE");

    // Save Feature Flags
    std::unordered_map<std::string, bool> flags;
    load_available_feature_flags([&flags](std::string flag) {
        std::string stripped_flag = strip_feature_flag_default(flag, NULL);
        flags[stripped_flag] = false;
    });
    {
        const char *enabled_flags = getenv("MCPI_FEATURE_FLAGS");
        if (enabled_flags == NULL) {
            IMPOSSIBLE();
        }
        std::istringstream enabled_flags_stream(enabled_flags);
        std::string flag;
        while (std::getline(enabled_flags_stream, flag, '|')) {
            if (flag.length() > 0) {
                flags[flag] = true;
            }
        }
    }
    for (auto &it : flags) {
        stream.write(it.first.c_str(), it.first.size() + 1);
        unsigned char val = it.second ? (unsigned char) 1 : (unsigned char) 0;
        stream.write((const char *) &val, 1);
    }

    // Finish
    stream.close();
    if (!stream.good()) {
        WARN("Failure While Saving Launcher Cache");
    }
}
