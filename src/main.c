#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "../includes/gui.h"
#include "../includes/audioNlyrics.h"




void Usage() {
    printf("Usage: ");
}

int main(int argc, char** args){

    // If no arguments given, just return
    if (argc < 2) {
        Usage();
        return 0;
    }


    // Default font is roboto, if user has specified a different font with --font flag, it uses that. handlefont() will check whether that font exists at all
    char* font = "roboto.ttf";
    if (argc>3) {
        for (int i = 3; i < argc-1; i++) {
            if (strcmp(args[i], "--font") == 0) {
                font = args[i+1];
                break;
            }
        }
    }


    // Start menu window, continuestatus is what state the user decided in the menu window
    char* SongName = args[1];
    char* ArtistName = args[2];
    // Checks if the lyrics exist, and hand the information to menu window
    CheckLRCLIB(SongName, ArtistName);
    if (strcmp(SongName, "") != 0 && strcmp(ArtistName, "") != 0) {
        // Checks if the song exists, and hand the information to menu window
        CheckYoutube(SongName, ArtistName);
    }

    int ContinueStatus = StartMenuWindow(font, SongName, ArtistName);


    if (ContinueStatus == 2) {
        exit(0);
    }

    bool DynamicStatus = true;
    // We hand startsongwindow the font and arguments and it will check whether the songs exist and start a series of parsing and the audio player
    // We just have to check if there are 2 arguments so it doesn't crash
    if (ContinueStatus == 0 && argc > 2) {
        StartSongWindow(font, args[1], args[2], DynamicStatus);
    } else if (ContinueStatus == 0 && argc <= 2) {
        exit(1);
        Usage();
    }
}
