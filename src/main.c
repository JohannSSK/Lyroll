#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#include "../includes/gui.h"
#include "../includes/audioNlyrics.h"

void Usage() {
    printf("Usage: lyroll <song_name> <artist_name> [--font font.ttf]\n");
    printf("\n");
    printf("Examples:\n");
    printf("  lyroll \"Would\" \"Alice In Chains\"\n");
    printf("  lyroll \"Savia\" \"Soen\" --font CinzelFont.ttf\n");
    printf("\n");
    printf("Options:\n");
    printf("  --font <file>    Use a custom font from assets/fonts/\n");
}

int main(int argc, char** args) {
    // If no arguments given, show usage and exit
    if (argc < 3) {
        Usage();
        return 1;
    }

    // Default font is roboto
    char* font = "roboto.ttf";
    if (argc > 3) {
        for (int i = 3; i < argc - 1; i++) {
            if (strcmp(args[i], "--font") == 0) {
                font = args[i + 1];
                break;
            }
        }
    }

    char* SongName = args[1];
    char* ArtistName = args[2];

    // Buffers for exact names from LRCLIB
    char exactArtist[256];
    char exactTrack[256];

    // Check if lyrics exist on LRCLIB
    int found = CheckLRCLIB(SongName, ArtistName, exactArtist, sizeof(exactArtist), exactTrack, sizeof(exactTrack));
    if (found <= 0) {
        printf("No synced lyrics found for \"%s\" by \"%s\"\n", SongName, ArtistName);
        return 1;
    }

    // Check if song exists on YouTube
    CheckYoutube(SongName, ArtistName);
    if (strlen(SongName) == 0 || strlen(ArtistName) == 0) {
        printf("Song not found on YouTube\n");
        return 1;
    }

    // Show menu
    printf("SongName: %s\n", SongName);
    printf("ArtistName: %s\n", ArtistName);
    printf("Exact Artist from LRCLIB: %s\n", exactArtist);
    printf("Exact Track from LRCLIB: %s\n", exactTrack);
    fflush(stdout);

    int ContinueStatus = StartMenuWindow(font, SongName, ArtistName);

    if (ContinueStatus == 2) {
        printf("Exiting...\n");
        return 0;
    }

    if (ContinueStatus == 0) {
        bool DynamicStatus = true;
        StartSongWindow(font, exactArtist, exactTrack, DynamicStatus);
    }

    return 0;
}
