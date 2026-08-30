#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "../includes/gui.h"
#include "../includes/audioNlyrics.h"
#include "../includes/const.h"
#include "../includes/parsing.h"
#include "raylib.h"

int StartSongWindow(char* font, char* ArtistName, char* SongName, bool DynamicStatus) {
    // Create a window with the song name and artist name
    InitializeWindow();
    Font rayLibFont = HandleFont(font);

    // DownloadAudioMp3 will use yt-dlp to download the mp3 file
    char* mp3Path = DownloadAudioMp3(SongName, ArtistName);
    if (mp3Path == NULL) {
        printf("Failed to download audio\n");
        return 1;
    }

    // GetLyrics uses LRCLIB to download the lyrics
    char* LyricsPath = GetLyrics(ArtistName, SongName);
    if (LyricsPath == NULL) {
        printf("Failed to download lyrics\n");
        return 1;
    }

    // Count lines in the lyrics
    int TotalLinesHuman = CountLines(LyricsPath);
    char** LyricsLines = SeparateLyricsLines(LyricsPath, TotalLinesHuman);
    long int LineResetTimeStamps[MAX_LINES] = {0};
    long int* timestamps = SeparateTimeStamps(LyricsLines, TotalLinesHuman);
    for (int i = 0; i < TotalLinesHuman; i++) {
        LineResetTimeStamps[i] = timestamps[i];
    }

    long int TotalTime = GetSongDuration(LyricsPath);
    int* LineTimeLengths = CalculateLineTimeLengths(LineResetTimeStamps, TotalLinesHuman);
    long int ShouldBeTotalTime = 0;
    for (int i = 0; i < TotalLinesHuman; i++) {
        ShouldBeTotalTime += LineTimeLengths[i];
    }
    if (ShouldBeTotalTime != TotalTime) {
        printf("Warning: ShouldBeTotalTime (%ld) does not match TotalTime (%ld)\n", ShouldBeTotalTime, TotalTime);
    }

    if (DynamicStatus) {
        float BPM = 120.0;
        if (CheckAubioExists()) {
            BPM = GetBpmFromAubio(mp3Path);
            printf("BPM: %.2f\n", BPM);
        } else {
            printf("Warning: aubio not found, using default BPM of 120\n");
        }

        // Count words per line
        int WordsPerLineHuman[MAX_LINES];
        for (int i = 0; i < TotalLinesHuman; i++) {
            WordsPerLineHuman[i] = CalculateWordsPerLineHuman(LyricsLines[i]);
        }

        // Separate words
        char* WordsOfEachLine[MAX_LINES][MAX_WORDS_PER_LINE];
        for (int i = 0; i < TotalLinesHuman; i++) {
            if (WordsPerLineHuman[i] == 0) {
                WordsOfEachLine[i][0] = NULL;
                continue;
            }
            for (int j = 0; j < WordsPerLineHuman[i]; j++) {
                WordsOfEachLine[i][j] = SeparateWords(LyricsLines, i, j);
            }
        }

        // Calculate word time values
        int WordTimeValues[MAX_LINES][MAX_WORDS_PER_LINE];
        for (int i = 0; i < TotalLinesHuman; i++) {
            for (int j = 0; j < WordsPerLineHuman[i]; j++) {
                if (WordsOfEachLine[i][0] == NULL) {
                    WordTimeValues[i][j] = LineTimeLengths[i];
                    continue;
                }
                WordTimeValues[i][j] = CalculateWordTimeValues(WordsOfEachLine[i][j], BPM);
            }
        }

        // Shrink or fill gaps
        int SumOfValues = 0;
        for (int i = 0; i < TotalLinesHuman; i++) {
            if (WordsPerLineHuman[i] == 0) {
                continue;
            }

            SumOfValues = 0;
            for (int j = 0; j < WordsPerLineHuman[i]; j++) {
                SumOfValues += WordTimeValues[i][j];
            }
            if (SumOfValues > LineTimeLengths[i]) {
                ShrinkWordTimeValues(WordTimeValues[i], WordsPerLineHuman[i], LineTimeLengths[i]);
            }

            SumOfValues = 0;
            for (int j = 0; j < WordsPerLineHuman[i]; j++) {
                SumOfValues += WordTimeValues[i][j];
            }
            if (SumOfValues > LineTimeLengths[i]) {
                printf("SumOfValues (%d) > LineTimeLengths[i] (%d)\n", SumOfValues, LineTimeLengths[i]);
                exit(1);
            }

            if (WordsPerLineHuman[i] > 0) {
                FillLineGap(WordTimeValues[i], WordsPerLineHuman[i], LineTimeLengths[i]);
            }

            SumOfValues = 0;
            for (int j = 0; j < WordsPerLineHuman[i]; j++) {
                SumOfValues += WordTimeValues[i][j];
            }
            if (SumOfValues < LineTimeLengths[i]) {
                printf("SumOfValues (%d) < LineTimeLength(%d)\n", SumOfValues, LineTimeLengths[i]);
                exit(1);
            }
        }

        // Calculate total words
        int TotalWordsHuman = 0;
        for (int i = 0; i < TotalLinesHuman; i++) {
            TotalWordsHuman += WordsPerLineHuman[i];
        }

        // Build word timestamps
        int WordTimeStamps[MAX_LINES * MAX_WORDS_PER_LINE];
        long int CurrentGlobalTime = 0;
        int WordIndex = 0;
        for (int i = 0; i < TotalLinesHuman; i++) {
            if (WordsPerLineHuman[i] == 0) continue;
            for (int j = 0; j < WordsPerLineHuman[i]; j++) {
                WordTimeStamps[WordIndex] = CurrentGlobalTime;
                CurrentGlobalTime += WordTimeValues[i][j];
                WordIndex++;
            }
        }

        int StartTime = (int)(GetTime() * 1000);
        PlayAudio(mp3Path);

        int PreviousLineValue = 1;
        int PreviousWordValue = 1;
        char Buffer[MAX_LINE_LENGTH] = "";
        int CurrentWordValueInLine = 1;

        while (!WindowShouldClose()) {
            long int CurrentTime = (int)(GetTime() * 1000 - StartTime);

            int CurrentLineValue = FindCurrentLineValue(CurrentTime, LineResetTimeStamps, TotalLinesHuman);
            int CurrentWordValue = FindCurrentWordValue(CurrentTime, WordTimeStamps, TotalWordsHuman);

            if (CurrentLineValue != PreviousLineValue) {
                PreviousLineValue = CurrentLineValue;
                if (WordsPerLineHuman[CurrentLineValue - 1] > 0) {
                    FlushLineBuffer(Buffer, WordsOfEachLine[CurrentLineValue - 1][0]);
                } else {
                    Buffer[0] = '\0';
                }
                CurrentWordValueInLine = 1;
            }

            if (CurrentWordValueInLine == 1) {
                CurrentWordValueInLine = 2;
                PreviousWordValue = CurrentWordValue;
            } else {
                if (CurrentWordValue != PreviousWordValue) {
                    PreviousWordValue = CurrentWordValue;
                    if (WordsPerLineHuman[CurrentLineValue - 1] > 0 &&
                        CurrentWordValueInLine - 1 < WordsPerLineHuman[CurrentLineValue - 1]) {
                        UpdateBufferWords(Buffer, WordsOfEachLine[CurrentLineValue - 1][CurrentWordValueInLine - 1]);
                    }
                    CurrentWordValueInLine++;
                }
            }

            BeginDrawing();
            int Resolutionx = GetScreenWidth();
            int Resolutiony = GetScreenHeight();

            PrintBuffer(Buffer, rayLibFont, Resolutionx, Resolutiony);
            DrawProgressBar(CurrentTime, TotalTime, Resolutionx, Resolutiony);
            EndDrawing();
        }

    } else {
        // Static mode
        char Buffer[MAX_LINE_LENGTH] = "";
        int StartTime = GetTime() * 1000;
        PlayAudio(mp3Path);

        int CurrentLine = 1;
        int PreviousLine = 1;

        while (!WindowShouldClose()) {
            long int CurrentTime = (GetTime() * 1000) - StartTime;
            int Resolutionx = GetScreenWidth();
            int Resolutiony = GetScreenHeight();

            CurrentLine = FindCurrentLineValue(CurrentTime, LineResetTimeStamps, TotalLinesHuman);
            if (CurrentLine != PreviousLine) {
                FlushLineBuffer(Buffer, LyricsLines[CurrentLine - 1]);
                PreviousLine = CurrentLine;
            }

            BeginDrawing();
            PrintBuffer(Buffer, rayLibFont, Resolutionx, Resolutiony);
            DrawProgressBar(CurrentTime, TotalTime, Resolutionx, Resolutiony);
            EndDrawing();
        }
    }

    free(mp3Path);
    free(LyricsPath);
    free(timestamps);
    free(LineTimeLengths);
    wait(NULL);
    return 0;
}
