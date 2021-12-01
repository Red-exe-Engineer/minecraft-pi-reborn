#include <libreborn/libreborn.h>

#include "game-mode.h"

#include <symbols/minecraft.h>

// Get Minecraft From Screen
static unsigned char *get_minecraft_from_screen(unsigned char *screen) {
    return *(unsigned char **) (screen + Screen_minecraft_property_offset);
}

// Redirect Create World Button To DemoLevelChooseScreen
#define WORLD_NAME "world"
static void SelectWorldScreen_tick_injection(unsigned char *screen) {
    bool create_world = *(bool *) (screen + SelectWorldScreen_should_create_world_property_offset);
    if (create_world) {
        // Get New World Name
        free(*demo_level_name);
        std::string new_name = (*SelectWorldScreen_getUniqueLevelName)(screen, WORLD_NAME);
        patch_address((void *) demo_level_name, (void *) strdup(new_name.c_str()));
        // Create DemoLevelChooseScreen
        unsigned char *new_screen = (unsigned char *) ::operator new(DEMO_LEVEL_CHOOSE_SCREEN_SIZE);
        ALLOC_CHECK(new_screen);
        (*DemoChooseLevelScreen)(new_screen);
        // Set Screen
        unsigned char *minecraft = get_minecraft_from_screen(screen);
        (*Minecraft_setScreen)(minecraft, new_screen);
        // Finish
        *(bool *) (screen + SelectWorldScreen_world_created_property_offset) = true;
    } else {
        (*SelectWorldScreen_tick)(screen);
    }
}
static void Touch_SelectWorldScreen_tick_injection(unsigned char *screen) {
    bool create_world = *(bool *) (screen + Touch_SelectWorldScreen_should_create_world_property_offset);
    if (create_world) {
        // Get New World Name
        free(*demo_level_name);
        std::string new_name = (*Touch_SelectWorldScreen_getUniqueLevelName)(screen, WORLD_NAME);
        patch_address((void *) demo_level_name, (void *) strdup(new_name.c_str()));
        // Create DemoLevelChooseScreen
        unsigned char *new_screen = (unsigned char *) ::operator new(DEMO_LEVEL_CHOOSE_SCREEN_SIZE);
        ALLOC_CHECK(new_screen);
        (*DemoChooseLevelScreen)(new_screen);
        // Set Screen
        unsigned char *minecraft = get_minecraft_from_screen(screen);
        (*Minecraft_setScreen)(minecraft, new_screen);
        // Finish
        *(bool *) (screen + Touch_SelectWorldScreen_world_created_property_offset) = true;
    } else {
        (*Touch_SelectWorldScreen_tick)(screen);
    }
}

void _init_game_mode_cpp() {
    // Hijack Create World Button
    patch_address(SelectWorldScreen_tick_vtable_addr, (void *) SelectWorldScreen_tick_injection);
    patch_address(Touch_SelectWorldScreen_tick_vtable_addr, (void *) Touch_SelectWorldScreen_tick_injection);
    // Make The DemoChooseLevelScreen Back Button Go To SelectWorldScreen Instead Of StartMenuScreen
    unsigned char demo_choose_level_screen_back_button_patch[4] = {0x05, 0x10, 0xa0, 0xe3}; // "mov r1, #0x5"
    patch((void *) 0x3a298, demo_choose_level_screen_back_button_patch);
    patch((void *) 0x39fa0, demo_choose_level_screen_back_button_patch);
}
// Reset Level Name
__attribute__((constructor)) static void _reset_level_name() {
    patch_address((void *) demo_level_name, NULL);
}
// Free Level Name
__attribute__((destructor)) static void _free_level_name() {
    free(*demo_level_name);
}
