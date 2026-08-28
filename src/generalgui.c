#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../includes/gui.h"
#include "../includes/const.h"
#include "raylib.h"

// Function handles font, checks whether font exists, and initializes it to be used later, returns the Font struct
Font HandleFont(char* font) {
    char path[256];
    snprintf(path, sizeof(path), "assets/fonts/%s", font);

    if (access(path, F_OK) != 0) {
        printf("Font not found: %s\n", path);
        exit(1);
    }

    Font rayLibFont = LoadFontEx(path, 200, NULL, 0);
    SetTextureFilter(rayLibFont.texture, TEXTURE_FILTER_BILINEAR);
    return rayLibFont;
}

// Initializes window with default starting settings.
void InitializeWindow() {
    // For raylib to be silent, otherwise it will flood the terminal with log messages
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // Makes the window resizable
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, PROGRAM_NAME);
    MaximizeWindow();
    SetTargetFPS(FPS);
}
