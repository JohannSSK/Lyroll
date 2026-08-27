#include <raylib.h>
#include <string.h>
#include <unistd.h>
#include "const.h"
#include "functions.h"


void MakeWindow(int count, long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN], char* font) {
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROGRAM_NAME);
    //MaximizeWindow();
    SetTargetFPS(FPS);;


    chdir("assets/fonts/");

    if (access(font, F_OK) != 0) {
        printf("Font not found: %s\n", font);
        exit(1);
    }

    char* pathToFont = realpath(font, NULL);
    printDebug("pathToFont:%s\n", pathToFont);
    chdir("../../");

    Font rayLibFont = LoadFontEx(pathToFont, 128, NULL, 0);


    SetTextureFilter(rayLibFont.texture, TEXTURE_FILTER_BILINEAR);

    // Times = milli
    // Strings = lyric[count]

    double startTime = GetTime(); // Starting time of the window
    int current = 0; // Current line we are at
    int length[MAX_LINES]; // length of each line in milliseconds
    int separation[MAX_LINES]; // Determines how long each words will appear for.

    // Calculate length for each line
    for (int i = 0; i < count-1; i++) {
        length[i] = milli[i+1] - milli[i];
        printDebug("milli[%d]:%lds\n", i, milli[i]);
        printDebug("length[%d]:%dms\n", i, length[i]);
    }

    //Calculate how many words are in each line

    char* words[MAX_LINES][32];
    int wordsInLine[MAX_LINES];
    FunctionWordsInLine(words, wordsInLine, count, lyric);



    for (int i = 0; i < count; i++) {
        for (int j = 0; j < wordsInLine[i]; j++) {
            printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
            printDebug("lyric[%d]:%s\n", i, lyric[i]);
            printDebug("words[%d][%d]:%s\n", i, j, words[i][j]);
        }
    }


    printDebug("Calculating separations...");





    // Calculate separations
    for (int i = 0; i < count - 1; i++) {
        if (wordsInLine[i] == 0) {
            wordsInLine[i] = 1;
            continue;
        }
        separation[i] = length[i] / wordsInLine[i];
        printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
        printDebug("separation[%d]:%dms\n", i, separation[i]);
    }

    // Calculate total words
    int totalWords = 0;
    for (int i = 0; i < count - 1; i++) {
        totalWords += wordsInLine[i];
    }
    printDebug("totalWords:%d\n", totalWords);


    // make an update milli with new values
    int time[totalWords];
    memset(time, 0, sizeof(time));
    int i = 1;
    int total = 1;
    while (i < count){
        for (int j = 1; j <= wordsInLine[i]; j++) {
            if (separation[i] == 0) {
                separation[i] = length[i];
            }
            printDebug("separation[%d]:%d, j:%d\n", i,separation[i], j);
            printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
            time[total] = time[total-1] + separation[i];
            printDebug("time[%d]:%dms\n", total, time[total]);
            total++;
        }
        i++;
    }





    char buffer[MAX_LINE_LEN] = "";
    int index = 0;
    int previous = 0;


    while (!WindowShouldClose()){
        double elapsed = (GetTime() - startTime) * 1000;
        printDebug("current time:%dms \n", (int)elapsed);

        current = findCurrentLine(elapsed, count, milli);

        // timer for buffer reset using milli
               if (current != previous) {
                   strcpy(buffer, "");
                   previous = current;
                   printDebug("buffer reset\n");
                   index = 0;
                   if (wordsInLine[current] > 1) {
                       printDebug("buffer updated THE FIRST WORD IS: %s\n", words[current][0]);
                       strcat(buffer, words[current][0]);
                   }

               }

               //timer for buffer update
               if (index < wordsInLine[current]){
                   int globalIndex = 0;
                   for (int k = 0; k < current; k++) {
                       globalIndex += wordsInLine[k];
                   }
                   globalIndex += index;


                   if (elapsed > time[globalIndex] ) {
                       strcat(buffer, " ");
                       strcat(buffer, words[current][index+1]);
                       printDebug("buffer updated: %s\n", buffer);
                       index ++;
                   }
               }




        printDebug("buffer:%s\n", buffer);

        BeginDrawing();
        ClearBackground(RAYWHITE);






            DrawTextEx(rayLibFont, buffer, (Vector2){100, 400}, 120, 2, BLACK);
           //
        EndDrawing();


    }
    CloseWindow();


}
