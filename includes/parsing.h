#ifndef PARSING_H
#define PARSING_H

#include <stddef.h>

// Word operations
char* SeparateWords(char** LyricsLines, int lineIndex, int wordIndex);
int CalculateWordsPerLineHuman(char* line);

// Buffer operations
void FlushLineBuffer(char* Buffer, char* String);
void UpdateBufferWords(char* Buffer, char* Word);

// Parsing functions
long int GetSongDuration(char* LyricsPath);
int CountLines(char* LyricsPath);
char** SeparateLyricsLines(char* LyricsPath, int TotalLinesHuman);

// Word timing
int CalculateWordTimeValues(char* word, float bpm);

// Timing adjustment
void ShrinkWordTimeValues(int* WordTimeValuesRow, int WordsPerLine, int LineTimeLength);
void FillLineGap(int* WordTimeValuesRow, int WordsPerLine, int LineTimeLength);

// Finding current positions
int FindCurrentLineValue(long int CurrentTime, long int* LineResetTimeStamps, int TotalLinesHuman);
int FindCurrentWordValue(long int CurrentTime, int* WordTimeStamps, int TotalWordsHuman);

// Parsing functions
long int* SeparateTimeStamps(char** lyrics, int TotalLinesHuman);
int* CalculateLineTimeLengths(long int* LineResetTimeStamps, int TotalLinesHuman);

// BPM detection
int CheckAubioExists(void);
float GetBpmFromAubio(char* mp3Path);

#endif
