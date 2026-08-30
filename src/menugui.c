#include "../includes/const.h"
#include "raylib.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../includes/gui.h"

#define OffsetY 200
#define ButtonSizeX 300
#define ButtonSizeY 50
#define TITLE_SIZE 80
#define TEXT_SELECTED 32
#define TEXT_N_SELECTED 24
#define ERROR_MSG_DURATION 3.0


Rectangle GetButtonRect(int index, int screenWidth) {
    int y = OffsetY + index * 60;
    return (Rectangle){
        screenWidth / 2.0f - ButtonSizeX / 2.0f,
        y,
        ButtonSizeX,
        ButtonSizeY
    };
}

int GetHoveredItem(Vector2 mouse, int options, int screenWidth) {
    for (int i = 0; i < options; i++) {
        Rectangle rect = GetButtonRect(i, screenWidth);
        if (CheckCollisionPointRec(mouse, rect)) {
            return i;
        }
    }
    return -1;
}

void DrawTitle(Font font, int screenWidth) {
    int size = MeasureTextEx(font, PROGRAM_NAME, TITLE_SIZE, 0).x;
    DrawTextEx(font, PROGRAM_NAME, (Vector2){screenWidth / 2.0f - size / 2.0f, 50}, TITLE_SIZE, 0, COLOR_TEXT);
}

void DrawMenuButton(Font font, int index, Vector2 mouse, int screenWidth, const char* text, bool enabled) {
    Rectangle rect = GetButtonRect(index, screenWidth);
    bool hover = CheckCollisionPointRec(mouse, rect) && enabled;

    if (hover) {
        DrawRectangle(rect.x, rect.y, rect.width, rect.height, COLOR_SURFACE);
    }

    int fontSize = hover ? TEXT_SELECTED : TEXT_N_SELECTED;
    Color textColor = enabled ? (hover ? COLOR_ACCENT : COLOR_TEXT_DIM) : (Color){80, 80, 80, 255};

    Vector2 textSize = MeasureTextEx(font, text, fontSize, 0);
    DrawTextEx(font, text, (Vector2){screenWidth / 2.0f - textSize.x / 2.0f, rect.y + (rect.height - fontSize) / 2}, fontSize, 0, textColor);
}

void DrawSongInfo(Font font, int screenWidth, int screenHeight, char* SongName, char* ArtistName) {
    char info[256];
    if (SongName != NULL && ArtistName != NULL) {
        snprintf(info, sizeof(info), "Song: %s  |  Artist: %s", SongName, ArtistName);
        DrawTextEx(font, info, (Vector2){20, screenHeight - 80}, 20, 0, COLOR_TEXT_DIM);
    }
}

void DrawError(Font font, int screenWidth, int screenHeight, const char* error, double timeElapsed, bool* errorActive) {
    if (!*errorActive) return;

    if (timeElapsed > ERROR_MSG_DURATION) {
        *errorActive = false;
        return;
    }

    char msg[256];
    snprintf(msg, sizeof(msg), "⚠ %s", error);
    Vector2 size = MeasureTextEx(font, msg, 24, 0);
    DrawTextEx(font, msg, (Vector2){screenWidth / 2.0f - size.x / 2.0f, 140}, 24, 0, COLOR_ERROR);
}

int StartMenuWindow(char* font, char* SongName, char* ArtistName) {
    InitializeWindow();
    Font rayLibFont = HandleFont(font);

    int options = 3;
    char* menuItems[] = {"Play Song", "Settings", "Exit"};

    bool errorActive = false;
    double errorTime = 0.0;
    char errorMessage[256] = "";
    bool playEnabled = (SongName != NULL && ArtistName != NULL);

    while (!WindowShouldClose()) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        Vector2 mouse = GetMousePosition();

        int hovered = GetHoveredItem(mouse, options, screenWidth);

        if (hovered == 0 && !playEnabled) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                strcpy(errorMessage, "Song not found in database");
                errorActive = true;
                errorTime = 0.0;
            }
        } else if (hovered >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (hovered == 0 && playEnabled) {
                // Play
                UnloadFont(rayLibFont);
                CloseWindow();
                return 0;
            } else if (hovered == 1) {
                // Settings
                UnloadFont(rayLibFont);
                CloseWindow();
                return 1;
            } else if (hovered == 2) {
                // Exit
                UnloadFont(rayLibFont);
                CloseWindow();
                return 2;
            }
        }

        if (errorActive) {
            errorTime += GetFrameTime();
        }

        BeginDrawing();
        ClearBackground(COLOR_BG);

        DrawTitle(rayLibFont, screenWidth);
        DrawError(rayLibFont, screenWidth, screenHeight, errorMessage, errorTime, &errorActive);
        DrawSongInfo(rayLibFont, screenWidth, screenHeight, SongName, ArtistName);

        for (int i = 0; i < options; i++) {
            bool enabled = (i == 0) ? playEnabled : true;
            DrawMenuButton(rayLibFont, i, mouse, screenWidth, menuItems[i], enabled);
        }

        DrawTextEx(rayLibFont, "Click a menu option", (Vector2){20, screenHeight - 40}, 16, 0, COLOR_TEXT_DIM);
        EndDrawing();
    }

    UnloadFont(rayLibFont);
    CloseWindow();
    return -1;
}
