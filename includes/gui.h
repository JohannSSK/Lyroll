#ifndef GUI_H
#define GUI_H

#include "raylib.h"

Font HandleFont(char* font);
void InitializeWindow();
Rectangle GetButtonRect(int index, int screenWidth);
int GetHoveredItem(Vector2 mouse, int options, int screenWidth);
void DrawTitle(Font font, int screenWidth);
int StartMenuWindow(char* font, char* SongName, char* ArtistName);
int StartSongWindow(char* font, char* song, char* artist, bool DynamicStatus);

#endif
