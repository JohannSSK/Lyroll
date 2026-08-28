#include "../includes/gui.h"
#include "raylib.h"

int StartSongWindow(char* font, char* SongName, char* ArtistName, bool DynamicStatus) {
    // Create a window with the song name and artist name
    InitializeWindow();
    HandleFont(font);
    // DownloadAudioMp3 will use yt-dlp to download the mp3 file of the song in assets/audio, and it returns the path
    // we will use to run that .mp3 file
    char* mp3Path = DownloadAudioMp3(SongName,ArtistName);
    if (mp3Path == NULL) {
        printf("Failed to download audio\n");
        return 1;
    }
    // DownloadLyrics works accordingly, it uses LRCLIB to download the lyrics... They require a lot of parsing however. we can't use them as is, but we keep their paths
    char* LyricsPath = DownloadLyrics(SongName, ArtistName);
    if (LyricsPath == NULL) {
        printf("Failed to download lyrics\n");
        return 1;
    }

    // This function tells us how many lines are in the whole song, 0 IS COUNTED AS A NUMBER, so it returns lines-1 in human numbers
    int TotalLines = CountLines(LyricsPath);
    // This function takes in the lyrics lines themselves, whole sentences
    char* LyricsLines[MAX_LINES] = SeparateLyricsLines(LyricsPath, TotalLines);
    // This Function will find the lyrics we want and put it's timestamps in ms in LineResetTimeStamps
    long int LineResetTimeStamps[MAX_LINES] = SeparateTimeStamps(LyricsPath, LyricsLines); // 0 if line doesn't have
    // This function returns the song duration from the file
    long int TotalTime = GetSongDuration(LyricsPath);

    // This function calculates how long each line will be showing up for, again in ms
    int LineTimeLengths[MAX_LINES] = CalculateLineTimeLengths(LineResetTimeStamps);
    long int ShouldBeTotalTime = 0;
    for (int i = 0; i < TotalLines; i++) {
        ShouldBeTotalTime += LineTimeLengths[i];
    }
    if (ShouldBeTotalTime != TotalTime) {
        printf("Warning: ShouldBeTotalTime (%d) does not match TotalTime (%d)\n", ShouldBeTotalTime, TotalTime);
    }



    if (DynamicStatus) {



        // This function will take the words and just counts them for each line
        // Will return 0 if there are no words
        int WordsPerLine[MAX_LINES];
        for (int i = 0; i < TotalLines; i++) {
            WordsPerLine[i] = CalculateWordsPerLine(LyricsLines[i]);
        }


        // This function takes the sentences we mentioned earlier and separates them into words
        // Should return NULL when there are no more words to separate
        char* WordsOfEachLine[MAX_LINES][MAX_WORDS_PER_LINE];
        for (int i = 0; i < TotalLines; i++) {
            if (WordsPerLine[i] == 0) {
                WordsOfEachLine[i][0] = NULL;
                continue;
            }
            for (int j = 0; j < WordsPerLine[i]; j++) {
                WordsOfEachLine[i][j] = SeparateWords();
            }
        }

        // This takes the words, and gives each word an ms value based on the letters in it.
        // Could be NULL, Returns 0 if so
        int WordTimeValues[MAX_LINES][MAX_WORDS_PER_LINE];
        for (int i = 0; i < TotalLines; i++) {
            for (int j = 0; j < WordsPerLine[i]; j++) {
                if (WordsOfEachLine[i][0] == NULL) {
                    WordTimeValues[i][j] = LineTimeLengths[i]; // if word is NULL, set to line time length
                    continue;
                }
                WordTimeValues[i][j] = CalculateWordTimeValues(WordsOfEachLine[i][j]); // in ms, how long a word should stay for, individually
            }
        }

        // Here we handle any errors that may occur if the line stays for less than the calculate word time values function counted, so we call shrink word time values which
        // shrinks them linearly until they fit it in the time they are supposed to
        int SumOfValues = 0;
        for (int i = 0; i < TotalLines; i++) {
            SumOfValues = 0;
            for (int j = 0; j < WordsPerLine[i]; j++) {
                SumOfValues += WordTimeValues[i][j];
            }
            if (SumOfValues > LineTimeLengths[i]) {
                ShrinkWordTimeValues(WordTimeValues[i], WordsPerLine[i], LineTimeLengths[i]);
            }


            for (int j = 0; j < WordsPerLine[i]; j++) {
                SumOfValues += WordTimeValues[i][j];
            }
            if (SumOfValues > LineTimeLengths[i]) {
                printf("SumOfValues (%d) > LineTimeLengths[i] (%d)\n ShrinkWordTimeValues has failed calculations.", SumOfValues, LineTimeLengths[i]);
                exit(1);
            }

        //This is going to take the WordTimeValues for each line, and the time it's actually supposed to stay for, and fill in the percise millisecond gaps there where the last word
        //Will be showing for that long
        //Assume we have 3 words, first one shall stay for 200ms second for 300ms and third for 400ms, but the lyric itself stays for 3 whole seconds,
        //We obviously have a gap, we don't want the lyrics to go out of sync with the audio, so we make the third word stay for 3 seconds - the remaining 200ms and 300ms
        // As we said, it's better for lyrics to appear before they are said, than after
            FillLineGap(WordTimeValues[i], WordsPerLine[i], LineTimeLengths[i]);
            if (SumOfValues < LineTimeLengths[i]) {
                printf("SumOfValues (%d) < LineTimeLength(%d) \nFillLineGap has failed calculations.", SumOfValues, LineTimeLengths[i]);
                exit(1);
            }

        }

        //So now we have the table WordTimeValues, which shows percisely how many seconds each word should be appearing for, to make it easier we make these into timestamps, which is pretty easy
        //We just calculate the sum of each previous word time. First we need the total words
        int TotalWords = 0;
        for (int i = 0; i < TotalLines; i++) {
            TotalWords += WordsPerLine[i];
        }


        //Now, we just iterate through the WordTimeValues table, and calculate the timestamp for each word
        int WordTimeStamps[MAX_LINES * MAX_WORDS_PER_LINE];
        long int CurrentGlobalTime = 0;
        int WordIndex = 0;
        for (int i = 0; i < TotalLines; i++) {
            for (int j = 0; j < WordsPerLine[i]; j++) { // see how 0 is counted as a number? We talked about this
                WordTimeStamps[WordIndex] = CurrentGlobalTime;
                CurrentGlobalTime += WordTimeValues[i][j];
                WordIndex++;
            }
        }
        //Now, we have the WordTimeStamps table, which shows the timestamp for each word
        //And LineResetTimeStamps, which shows when each line should be flushed and show the next one
        //So that's all we need to show the lyrics in sync.
        //We will obviously use a buffer for that, which will clear when the line should be flushed
        //And then we will show the next line

        int StartTime = (int)(GetTime()*1000);
        int PreviousLineValue = -1; // 0 would mean the first line, but we start at -1 so we can detect when the line changes
        int PreviousWordValue = -1; // same thing
        char Buffer[MAX_LINE_LENGTH] = "";

        while (!WindowShouldClose()) {
            int CurrentTime = (int)(GetTime()*1000 - StartTime);

            // These will take the current time and based on it, will find which word and line we should be on.
            int CurrentLineValue = FindCurrentLineValue(CurrentTime, LineResetTimeStamps, TotalLines); // could be 0, which means the first line
            int CurrentWordValue = FindCurrentWordValue(CurrentTime, WordTimeStamps, TotalWords); // Same thing here

            // If the line has changed, Flush Line Buffer will delete the buffer, so we start clean. It will add the first word though, we always start with the first word already there
            // And later, if the word has changed, we update the buffer with the new word
            if (CurrentLineValue != PreviousLineValue) {
                PreviousLineValue = CurrentLineValue;
                FlushLineBuffer(Buffer, WordsOfEachLine[CurrentLineValue][0]);
            }
            if (CurrentWordValue != PreviousWordValue) {
                if (CurrentWordValue == 0) {
                    CurrentWordValue = PreviousWordValue;
                    // Remember, the buffer has already put the first word in, so we don't need to update it
                } else {
                    PreviousWordValue = CurrentWordValue;
                    // Update the buffer with the new word will also put a space after each word.
                    // It takes the current Word
                    UpdateBufferWords(Buffer, WordsOfEachLine[CurrentLineValue][CurrentWordValue], WordTimeStamps);
                }
            }
            // So now, the buffer shows exactly what the user is hearing, all that remains is to show it
            // Which isn't simple either, because we will calculate how big the buffer is, and make it fit the screen
            BeginDrawing();
            int Resolutionx = GetScreenWidth();
            int Resolutiony = GetScreenHeight();

            PrintBuffer(Buffer, Resolutionx, Resolutiony);
            DrawProgessBar(CurrentTime, TotalTime, Resolutionx, Resolutiony);
            EndDrawing();


        }



    }else{

        // This is so much simpler of course than the real time lyrics...
        // We already have LineResetTimeStamps and LyricsLines, so we can just use those to show the lyrics
        // But we will use a buffer here too so we can reuse the previous function which calculates how it fits etc

        char Buffer[MAX_LINE_LENGTH] = "";
        while (!WindowShouldClose()) {
            int Resolutionx = GetScreenWidth();
            int Resolutiony = GetScreenHeight();

            BeginDrawing();
            PrintBuffer(Buffer, Resolutionx, Resolutiony);
            EndDrawing();
        }
    }
}
