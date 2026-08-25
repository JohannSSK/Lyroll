#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_LINES 256
#define MAX_LINE_LEN 256
#define MAX_STR_LEN 64

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define fps 60

#define FONT_SIZE 60
#define TEXT_SPACING 2

#define AUDIO_PLAYER_PATH "/usr/bin/mpg123"
#define AUDIO_PLAYER_NAME "mpg123"
#define QUIET_FLAG "-q"

#define DEBUG 1

#if DEBUG
    #define printDebug(...) printf(__VA_ARGS__)
#else
    #define printDebug(...) ((void)0)
#endif

#endif
