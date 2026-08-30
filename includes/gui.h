#ifndef GUI_H
#define GUI_H

#include <raylib.h>
#include <stdbool.h>

// Window and font
Font HandleFont(char* font);
void InitializeWindow(void);

// Display
void PrintBuffer(char* Buffer, Font font, int ResolutionX, int ResolutionY);
void DrawProgressBar(long int CurrentTime, long int TotalTime, int ResolutionX, int ResolutionY);

// Menu
int StartMenuWindow(char* font, char* SongName, char* ArtistName);

// Song window
int StartSongWindow(char* font, char* ArtistName, char* SongName, bool DynamicStatus);

#endif
