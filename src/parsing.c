#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "../libs/cJSON/cJSON.h"


struct LyricData {
    char** lines;
    int* start_ms;
    int* end_ms;
    int count;
    int duration_ms;
};


// Generic... it's always the same don't worry about it
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    char **response = (char**)userp;
    size_t old_len = strlen(*response);
    char *new = realloc(*response, old_len + total + 1);
    if (!new) return 0;
    *response = new;
    memcpy(*response + old_len, contents, total);
    (*response)[old_len + total] = '\0';
    return total;
}


// Works with recursion, not the best way to do it,
// but it works and it's pretty simple
char* FindId(char* search_buffer, char* id_start, int limit)
{
    char* syncedLyrics;
    id_start = strstr(search_buffer, "\"id\":");

    if (id_start) {
        id_start += 5;
        syncedLyrics = strstr(search_buffer, "\"syncedLyrics\":");
        syncedLyrics += 15;
    }

    if (strncmp(syncedLyrics, "null", 4) == 0) {
        search_buffer += 150;
        search_buffer = strstr(search_buffer, "\"id\":");
        FindId(search_buffer, id_start, limit + 1);
    } else {
        return id_start;
    }

    if (limit > 20) {
        printf("No synced lyrics found");
        return NULL;
    }
}


// This is the main function that calls the other functions here. Will be called by main
// It takes the songname and artist name and returns the id of the lyrics so we can curl the good ones later...
// this whole thing is to just find a lyric file that surely contains syncedlyrics, otherwise we exit.
// I did notice that NOT having synced lyrics is VERY rare, even in songs that aren't that known at all...
// It still deserves to be there for protection
//curl_global_init(CURL_GLOBAL_DEFAULT);
// has to have been called before this, in main
struct LyricData GetLRCLIB(char* SongName, char* ArtistName)
{
    char encoded_buffer[500] = {0};
    int j = 0;

    for (int i = 0; SongName[i] && j < sizeof(encoded_buffer) - 1; i++) {
        if (SongName[i] == ' ') {
            encoded_buffer[j++] = '%';
            encoded_buffer[j++] = '2';
            encoded_buffer[j++] = '0';
        } else {
            encoded_buffer[j++] = SongName[i];
        }
    }
    encoded_buffer[j] = '\0';
    strcpy(SongName, encoded_buffer);

    strcpy(encoded_buffer, "");
    j = 0;

    for (int i = 0; ArtistName[i] && j < sizeof(encoded_buffer) - 1; i++) {
        if (ArtistName[i] == ' ') {
            encoded_buffer[j++] = '%';
            encoded_buffer[j++] = '2';
            encoded_buffer[j++] = '0';
        } else {
            encoded_buffer[j++] = ArtistName[i];
        }
    }
    encoded_buffer[j] = '\0';
    strcpy(ArtistName, encoded_buffer);

    char url[500] = "";
    snprintf(url, sizeof(url), "https://lrclib.net/api/search?q=%s+%s", SongName, ArtistName);

    CURL* curl = curl_easy_init();
    char* raw_api_response = malloc(1);
    raw_api_response[0] = '\0';
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw_api_response);
    curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    char* api_response_copy = raw_api_response;

    char* id_start;
    char* id_pointer = FindId(raw_api_response, id_start, 0);

    if (id_pointer == NULL) {
        printf("error, didn't find synced lyrics");
        exit(1);
    }

    char* id_string = malloc(10);
    strncpy(id_string, id_pointer, 8);
    id_string[9] = '\0';

    char* extracted_yaml = malloc(10000);
    struct LyricData song_lyrics;

    cJSON *root = cJSON_Parse(api_response_copy);
    cJSON *item;

    cJSON_ArrayForEach(item, root) {
        cJSON *id_json = cJSON_GetObjectItem(item, "id");

        if (id_json && (*id_json).valueint == atoi(id_string)) {
            cJSON *lyricsfile = cJSON_GetObjectItem(item, "lyricsfile");
            char *lyricsdata = cJSON_GetStringValue(lyricsfile);
            strcpy(extracted_yaml, lyricsdata);
            break;
        }
    }

    cJSON_Delete(root);

    char* duration_start_ptr = strstr(extracted_yaml, "duration_ms: ");
    duration_start_ptr += 13;
    char* duration_end_ptr = strstr(duration_start_ptr, "\n");

    int duration_length = duration_end_ptr - duration_start_ptr;
    char* duration_string = malloc(100);

    strncpy(duration_string, duration_start_ptr, duration_length);
    duration_string[duration_length] = '\0';
    song_lyrics.duration_ms = atoi(duration_string);

    char* lines_section_buffer = malloc(10000);
    char* start = strstr(extracted_yaml, "lines:");
    char* end = strstr(start, "\nplain:");
    int len = end - start;
    strncpy(lines_section_buffer, start, len);
    lines_section_buffer[len] = '\0';

    char* line_buffer = malloc(10000);
    char* search_ptr = lines_section_buffer;

    song_lyrics.lines = malloc(100 * sizeof(char*));
    song_lyrics.start_ms = malloc(100 * sizeof(int));
    song_lyrics.end_ms = malloc(100 * sizeof(int));
    song_lyrics.count = 0;

    while ((start = strstr(search_ptr, "- text:")) != NULL) {
        song_lyrics.count++;

        search_ptr = start + 7;  // Move past "- text:"
        end = strstr(search_ptr, "- text:");  // Find the next one

        if (!end) end = lines_section_buffer + strlen(lines_section_buffer);  // If none, go to end

        while (end > start && *(end - 1) != '\n') end--;

        int len = end - start;
        strncpy(line_buffer, start, len);
        line_buffer[len] = '\0';

        // Extract text
        char* text_start = strstr(line_buffer, "- text:");
        text_start += 8;
        char* text_end = strstr(text_start, "\n");
        int text_len = text_end - text_start;
        char* text = malloc(text_len + 1);
        strncpy(text, text_start, text_len);
        text[text_len] = '\0';

        // Extract start_ms
        char* ms_start = strstr(line_buffer, "start_ms: ");
        int start_ms;

        if (ms_start != NULL) {
            ms_start += 10;
            char* ms_end = strstr(ms_start, "\n");
            int ms_len = ms_end - ms_start;
            char ms_temp[20];
            strncpy(ms_temp, ms_start, ms_len);
            ms_temp[ms_len] = '\0';
            start_ms = atoi(ms_temp);
        } else {
            start_ms = song_lyrics.end_ms[song_lyrics.count - 2];
        }

        // Extract end_ms
        char* end_start = strstr(line_buffer, "end_ms: ");
        int end_ms;

        if (end_start != NULL) {
            end_start += 8;
            char* end_end = strstr(end_start, "\n");
            int end_len = end_end - end_start;
            char end_temp[20];
            strncpy(end_temp, end_start, end_len);
            end_temp[end_len] = '\0';
            end_ms = atoi(end_temp);
        } else {
            end_ms = song_lyrics.duration_ms;
        }

        // printf("text: %s | start_ms: %d | end_ms: %d\n", text, start_ms, end_ms);
        // fflush(stdout);

        // Store in struct
        song_lyrics.lines[song_lyrics.count - 1] = text;
        song_lyrics.start_ms[song_lyrics.count - 1] = start_ms;
        song_lyrics.end_ms[song_lyrics.count - 1] = end_ms;

        search_ptr = end;  // Move forward for next loop
    }

    // Free everything that's no longer needed
    free(raw_api_response);
    free(id_string);
    free(duration_string);
    free(extracted_yaml);
    free(line_buffer);

    return song_lyrics;
}

// CALL LIKE:
// struct LyricData Lyrics = GetLRCLIB(song, artist);
