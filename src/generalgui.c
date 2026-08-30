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

void PrintBuffer(char* Buffer, Font font, int ResolutionX, int ResolutionY) {
    if (Buffer == NULL || Buffer[0] == '\0') return;

    int fontSize = 200;
    Vector2 textSize = MeasureTextEx(font, Buffer, fontSize, 1);
    float maxWidth = ResolutionX * 0.85;

    // Shrink font until text fits within 85% of screen width
    while (textSize.x > maxWidth && fontSize > 20) {
        fontSize -= 4;
        textSize = MeasureTextEx(font, Buffer, fontSize, 1);
    }

    // Center the text
    Vector2 position = {
        (ResolutionX - textSize.x) / 2.0f,
        (ResolutionY - textSize.y) / 2.0f
    };
    printf("%s\n", Buffer);

    DrawTextEx(font, Buffer, position, fontSize, 1, COLOR_TEXT);
}


void DrawProgressBar(long int CurrentTime, long int TotalTime, int ResolutionX, int ResolutionY) {
    if (TotalTime <= 0) return;

    // Calculate progress (0.0 to 1.0)
    float progress = (float)CurrentTime / (float)TotalTime;
    if (progress > 1.0f) progress = 1.0f;
    if (progress < 0.0f) progress = 0.0f;

    int barWidth = ResolutionX - 200;
    int barHeight = 8;
    int x = 100;
    int y = ResolutionY - 50;

    // Background
    DrawRectangleRounded((Rectangle){x, y, barWidth, barHeight}, 0.5f, 10, COLOR_PROGRESS_BG);

    // Foreground (filled portion)
    int filledWidth = (int)(barWidth * progress);
    if (filledWidth > 0) {
        DrawRectangleRounded((Rectangle){x, y, filledWidth, barHeight}, 0.5f, 10, COLOR_PROGRESS_FG);
    }

    // Optional: time text
    char timeText[32];
    int minutes = CurrentTime / 60000;
    int seconds = (CurrentTime % 60000) / 1000;
    snprintf(timeText, sizeof(timeText), "%02d:%02d", minutes, seconds);
    DrawTextEx(GetFontDefault(), timeText, (Vector2){x, y - 25}, 16, 1, COLOR_TEXT_DIM);
}
