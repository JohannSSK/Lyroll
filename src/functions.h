#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <stdio.h>
#include <stdlib.h>
#include "const.h"

// Audio with mpg123
int initMusic(char* absolute_path);


// Preparing lyrics and timings
void stats(FILE* file,char* line, char* id, char* artist, char* album, char* title, char* length);

void extractLyrics(FILE* file, char lyric[MAX_LINES][MAX_LINE_LEN], char* line, int* count, int* minutes, float* seconds);

void lyricsMilli(long int* milli, int* minutes, float* seconds, int* count);


// timings
int findCurrentLine(long int elapsed, int count, long int* milli);

void FunctionWordsInLine(char* words[MAX_LINES][32], int wordsInLine[MAX_LINES], int count, char lyric[MAX_LINES][MAX_LINE_LEN]);

// GUI
void MakeWindow(int count, long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN]);

#endif
