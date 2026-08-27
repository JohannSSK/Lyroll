#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <stdio.h>
#include <stdlib.h>
#include "const.h"

// Audio with mpg123
int initMusic(char* absolute_path);


// Preparing files, lyrics and timings

int checkIfFilesAreValid(int argumentCount, char* lyricsFile, char* mp3File, char* pathToLyricsFile, char* pathToMp3File);

void stats(FILE* file, char* id, char* artist, char* album, char* title, char* length);

void extractLyrics(FILE* file, char lyric[MAX_LINES][MAX_LINE_LEN], int* count, int* minutes, float* seconds);

void lyricsMilli(long int* milli, int* minutes, float* seconds, int* count);


// timings
int findCurrentLine(long int elapsed, int count, long int* milli);

void FunctionWordsInLine(char* words[MAX_LINES][32], int wordsInLine[MAX_LINES], int count, char lyric[MAX_LINES][MAX_LINE_LEN]);

// GUI
void MakeWindowRealTime(int count, long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN], char* font);
void MakeWindowStatic(int count,long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN], char* font);

#endif
