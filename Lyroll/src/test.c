#include <raylib.h>
#include <string.h>
#include "const.h"

int main(int argc, char** args) {
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROGRAM_NAME);
    MaximizeWindow();
    SetTargetFPS(FPS);

    Font font = LoadFontEx("../assets/fonts/roboto.ttf", 128, NULL, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    char* words[] = {"This", "is", "a", "test", "for", "you"};
    int wordCount = 6;
    long int separate = 500;
    char buffer[1024] = "";
    int wordIndex = 0;
    double wordStartTime = GetTime() * 1000;

    while (!WindowShouldClose()) {
        double elapsed = (GetTime() * 1000 - wordStartTime);

        if (elapsed >= separate && wordIndex < wordCount) {
            if (wordIndex > 0) strcat(buffer, " ");
            strcat(buffer, words[wordIndex]);
            wordIndex++;
            wordStartTime = GetTime() * 1000;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTextEx(font, buffer, (Vector2){100, 400}, 120, 2, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
