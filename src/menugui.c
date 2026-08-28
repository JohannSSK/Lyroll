#include "../includes/const.h"
#include "raylib.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../includes/gui.h"

#define OffsetY 200
#define ButtonSizeX 300
#define ButtonSizeY 50
#define TITLE_SIZE 80
#define TEXT_SELECTED 32
#define TEXT_N_SELECTED 24


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

void DrawMenuButton(Font font, int index, Vector2 mouse, int screenWidth, const char* text) {
    Rectangle rect = GetButtonRect(index, screenWidth);
    bool hover = CheckCollisionPointRec(mouse, rect);

    if (hover) {
        DrawRectangle(rect.x, rect.y, rect.width, rect.height, COLOR_SURFACE);
    }

    int fontSize = hover ? TEXT_SELECTED : TEXT_N_SELECTED;
    Color textColor = hover ? COLOR_ACCENT : COLOR_TEXT_DIM;

    Vector2 textSize = MeasureTextEx(font, text, fontSize, 0);
    DrawTextEx(font, text, (Vector2){screenWidth / 2.0f - textSize.x / 2.0f, rect.y + (rect.height - fontSize) / 2}, fontSize, 0, textColor);
}

// Start menu window, continuestatus is what state the user decided in the menu window
int StartMenuWindow(char* font) {
    InitializeWindow();
    Font rayLibFont = HandleFont(font);

    int options = 3;
    char* menuItems[] = {"Play Song", "Settings", "Exit"};

    while (!WindowShouldClose()) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        Vector2 mouse = GetMousePosition();

        int hovered = GetHoveredItem(mouse, options, screenWidth);

        if (hovered >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int result = hovered;
            UnloadFont(rayLibFont);
            CloseWindow();
            return result;
        }

        BeginDrawing();
        ClearBackground(COLOR_BG);

        DrawTitle(rayLibFont, screenWidth);

        for (int i = 0; i < options; i++) {
            DrawMenuButton(rayLibFont, i, mouse, screenWidth, menuItems[i]);
        }

        DrawTextEx(rayLibFont, "Click a menu option", (Vector2){20, screenHeight - 40}, 16, 0, COLOR_TEXT_DIM);
        EndDrawing();
    }

    UnloadFont(rayLibFont);
    CloseWindow();
    return -1;
}
