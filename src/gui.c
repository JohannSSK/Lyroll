#include <raylib.h>
#include <string.h>
#include <unistd.h>
#include "const.h"
#include "functions.h"



#define COLOR_GRADIENT_TOP (Color){22, 22, 30, 255}
#define COLOR_GRADIENT_BOTTOM (Color){22, 22, 30, 255}
#define COLOR_TEXT (Color){255, 255, 255, 255}
#define COLOR_TEXT_DIM (Color){160, 160, 175, 255}
#define COLOR_PROGRESS_BG (Color){40, 40, 52, 255}
#define COLOR_PROGRESS_FG (Color){80, 160, 255, 255}
#define COLOR_ACCENT (Color){130, 130, 200, 255}




Font handleFont(char* font) {


    chdir("assets/fonts/");

    if (access(font, F_OK) != 0) {
        printf("Font not found: %s\n", font);
        exit(1);
    }

    char* pathToFont = realpath(font, NULL);
    printDebug("pathToFont:%s\n", pathToFont);
    chdir("../../");
     Font rayLibFont = LoadFontEx(pathToFont, 200, NULL, 0);
     SetTextureFilter(rayLibFont.texture, TEXTURE_FILTER_BILINEAR);

    return rayLibFont;

}

int* calculateSeparations(int* wordsInLine, int count, int* length) {

    int* separation = calloc(count, sizeof(int));

    for (int i = 0; i < count - 1; i++) {
        if (wordsInLine[i] == 0) {
            wordsInLine[i] = 1;
            separation[i] = length[i];
            continue;
        }
        separation[i] = length[i] / wordsInLine[i];
        printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
        printDebug("separation[%d]:%dms\n", i, separation[i]);

    }
    return separation;
}

void initializeWindow(void){

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROGRAM_NAME);
    MaximizeWindow();
    SetTargetFPS(FPS);;



}


void Styling(int width, int height, Font rayLibFont, char* buffer) {

    DrawRectangleGradientV(0, 0, width, height, COLOR_GRADIENT_TOP, COLOR_GRADIENT_BOTTOM);

    if (strcmp(buffer, "") == 0) return;

    int size = 200;
    double maxsize = width*0.8;
    Vector2 textSize = MeasureTextEx(rayLibFont, buffer, size, 1);
    while (textSize.x > maxsize) {
        size -= 10;
        textSize = MeasureTextEx(rayLibFont, buffer, size, 1);
    }

    Vector2 position = {width/2.0 - textSize.x/2, height/2.0 - textSize.y/2};

    DrawTextEx(rayLibFont, buffer, position, size, 1, COLOR_TEXT);


}

void progresBar(int width, int height, int elapsed, char* total) {


    int minutes, seconds;


    sscanf(total, "%d:%d", &minutes, &seconds);

    long int totalMilliseconds = minutes * 60000 + seconds * 1000;
    int progress = (int)((double)elapsed / totalMilliseconds * 100);



    Rectangle progressBarBackground = {100, height - 100, width - 200, 10};
    Rectangle progressBarForeground = {100, height - 100, (width-200) * progress / 100, 10};

    DrawRectangleRounded(progressBarBackground, 0.3, 10, COLOR_PROGRESS_BG);
    DrawRectangleRounded(progressBarForeground, 0.3, 10, COLOR_PROGRESS_FG);

}

void MakeWindowRealTime(int count, long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN], char* font, char* totalLength) {



    //Initializes window
    initializeWindow();


    // Finds font and returns it to be used for prints
    Font rayLibFont = handleFont(font);



    double startTime = GetTime(); // Starting time of the window
    int current = 0; // Current line we are at
    int length[MAX_LINES]; // length of each line in milliseconds
    int* separation = NULL; // Determines how long each words will appear for.

    // Calculate how long each line will be appearing for in ms
    for (int i = 0; i < count-1; i++) {
        length[i] = milli[i+1] - milli[i];
        printDebug("milli[%d]:%lds\n", i, milli[i]);
        printDebug("length[%d]:%dms\n", i, length[i]);
    }

    //Calculate how many words are in each line

    char* words[MAX_LINES][32]; //  will contain all words in all lyrisc
    int wordsInLine[MAX_LINES]; //  How many words in each lyric
    FunctionWordsInLine(words, wordsInLine, count, lyric);


    // Debugging
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < wordsInLine[i]; j++) {
            printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
            printDebug("lyric[%d]:%s\n", i, lyric[i]);
            printDebug("words[%d][%d]:%s\n", i, j, words[i][j]);
        }
    }




    printDebug("Calculating separations...");

    // Calculate separations
    separation = calculateSeparations(wordsInLine,count,length);

    // Calculate total words
    int totalWords = 0;
    for (int i = 0; i < count; i++) {
        totalWords += wordsInLine[i];
    }


    printDebug("It's not crashing now");
    fflush(stdout);
    printDebug("totalLength ITS HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE: %s\n", totalLength);
    fflush(stdout);



    // make an update milli with new values with the words in between.
    int time[totalWords];
    printDebug("time:%d\n", totalWords);
    memset(time, 0, sizeof(time));
    int i = 1;
    int total = 1;
    while (i < count){
        printDebug("i:%d\n", i);
        for (int j = 1; j <= wordsInLine[i]; j++) {
            printDebug("j:%d\n", j);
            if (separation[i] == 0) {
                separation[i] = length[i];
            }
            printDebug("separation[%d]:%d, j:%d\n", i,separation[i], j);
            printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
            time[total] = time[total-1] + separation[i];
            printDebug("time[%d]:%dms\n", total, time[total]);
            total++;
            printDebug("total:%d\n", total);
            printDebug("totalWords:%d\n", totalWords);
        }
        i++;
    }

    printDebug("It's probably going to crash now!!!!!!!!!!");
    fflush(stdout);
    printDebug("totalLength ITS HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE: %s\n", totalLength);
    fflush(stdout);

    char buffer[MAX_LINE_LEN] = "";
    int index = 0;
    int previous = 0;


    while (!WindowShouldClose()){
        // calculates current time in ms since window opened
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



               if (index < wordsInLine[current]-1){
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


        BeginDrawing();
        ClearBackground(RAYWHITE);



            Styling(GetScreenWidth(),GetScreenHeight(),rayLibFont,buffer);


            progresBar(GetScreenWidth(), GetScreenHeight(), elapsed, totalLength);


        EndDrawing();


    }

    //Cleaning up
    free(separation);
    CloseWindow();


}




void MakeWindowStatic(int count, long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN], char* font, char* totalLength) {


    //Initializes window
    initializeWindow();


    // Finds font and returns it to be used for prints
    Font rayLibFont = handleFont(font);



    double startTime = GetTime(); // Starting time of the window
    int current = 0; // Current line we are at

    char buffer[MAX_LINE_LEN] = "";


    while (!WindowShouldClose()){

        // calculates current time in ms since window opened
        double elapsed = (GetTime() - startTime) * 1000;
        printDebug("current time:%dms \n", (int)elapsed);

        current = findCurrentLine(elapsed, count, milli);

        strcpy(buffer, lyric[current]);


        BeginDrawing();
        ClearBackground(RAYWHITE);

            Styling(GetScreenWidth(),GetScreenHeight(),rayLibFont,buffer);
            progresBar(GetScreenWidth(), GetScreenHeight(), elapsed, totalLength);

        EndDrawing();


    }

    //Cleaning up
    CloseWindow();
}
