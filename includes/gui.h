#ifndef GUI_H
#define GUI_H

#include "raylib.h"

Font HandleFont(char* font);
void InitializeWindow();
Rectangle GetButtonRect(int index, int screenWidth);
int GetHoveredItem(Vector2 mouse, int options, int screenWidth);
void DrawTitle(Font font, int screenWidth);
void DrawMenuButton(Font font, int index, Vector2 mouse, int screenWidth, const char* text);
int StartMenuWindow(char* font);
int StartSongWindow(char* font, char* song, char* artist, bool DynamicStatus);  // ← ADD THIS

#endif
