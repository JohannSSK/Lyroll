#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "../libs/cJSON/cJSON.h"
#include <unistd.h>
#include <sys/wait.h>

void PlayAudio(char* mp3Path) {
    int pid = fork();

    if (pid == 0) {
        // Child process: play audio
        execlp("mpg123", "mpg123", "-q", mp3Path, NULL);
        perror("execlp failed");
        exit(1);
    } else if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    // Parent: return immediately (don't wait)
    // The child runs in the background
}

// For curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
size_t callback(void* data, size_t size, size_t nmemb, void* buffer) {
    size_t total = size * nmemb;
    char* buf = (char*)buffer;
    size_t current_len = strlen(buf);
    size_t max_len = 5242879;  // 5MB - 1

    if (current_len + total < max_len) {
        memcpy(buf + current_len, data, total);
        buf[current_len + total] = '\0';
    }
    return total;
}

int CheckLRCLIB(char* SongName, char* ArtistName, char* exactArtist, size_t artistBufSize, char* exactTrack, size_t trackBufSize) {
    printf("CheckLRCLIB: START\n");
    fflush(stdout);

    if (exactArtist == NULL || exactTrack == NULL) {
        printf("CheckLRCLIB: exactArtist or exactTrack is NULL\n");
        return -1;
    }
    if (SongName == NULL || ArtistName == NULL) {
        printf("CheckLRCLIB: SongName or ArtistName is NULL\n");
        return -1;
    }

    // URL-encode the search query properly
    char* encodedArtist = curl_easy_escape(NULL, ArtistName, 0);
    char* encodedSong = curl_easy_escape(NULL, SongName, 0);
    if (!encodedArtist || !encodedSong) {
        if (encodedArtist) curl_free(encodedArtist);
        if (encodedSong) curl_free(encodedSong);
        return -1;
    }

    char url[1024];
    snprintf(url, sizeof(url), "https://lrclib.net/api/search?artist_name=%s&track_name=%s&limit=5", encodedArtist, encodedSong);

    curl_free(encodedArtist);
    curl_free(encodedSong);

    char* response_buffer = malloc(5242880);
    if (!response_buffer) return -1;
    memset(response_buffer, 0, 5242880);

    CURL* curl = curl_easy_init();
    if (!curl) {
        free(response_buffer);
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    cJSON* json = cJSON_Parse(response_buffer);
    if (!json) {
        printf("CheckLRCLIB: Failed to parse JSON response\n");
        free(response_buffer);
        return -1;
    }

    int arraySize = cJSON_GetArraySize(json);
    for (int i = 0; i < arraySize; i++) {
        cJSON* item = cJSON_GetArrayItem(json, i);
        cJSON* synced = cJSON_GetObjectItem(item, "syncedLyrics");
        char* syncedStr = cJSON_GetStringValue(synced);

        if (syncedStr != NULL && strlen(syncedStr) > 0) {
            // Extract exact artist and track names
            cJSON* artistObj = cJSON_GetObjectItem(item, "artistName");
            cJSON* trackObj = cJSON_GetObjectItem(item, "trackName");
            if (artistObj && trackObj) {
                char* art = cJSON_GetStringValue(artistObj);
                char* trk = cJSON_GetStringValue(trackObj);
                if (art && trk) {
                    // Use strncpy with bounds checking
                    strncpy(exactArtist, art, artistBufSize - 1);
                    exactArtist[artistBufSize - 1] = '\0';
                    strncpy(exactTrack, trk, trackBufSize - 1);
                    exactTrack[trackBufSize - 1] = '\0';
                }
            }
            cJSON_Delete(json);
            free(response_buffer);
            return 1;  // found
        }
    }

    cJSON_Delete(json);
    free(response_buffer);
    return 0;  // not found
}

void CheckYoutube(char* SongName, char* ArtistName) {
    printf("CheckYoutube: START\n");
    fflush(stdout);

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "yt-dlp \"ytsearch:%s %s\" --get-id --get-title --no-warnings 2>/dev/null",
        ArtistName, SongName);

    printf("CheckYoutube: cmd: %s\n", cmd);
    fflush(stdout);

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        printf("Error with opening pipe in CheckYoutube\n");
        return;
    }

    char VideoId[64];
    char ExactTitle[256];

    if (fgets(ExactTitle, sizeof(ExactTitle), fp) == NULL) {
        printf("CheckYoutube: No exact title\n");
        pclose(fp);
        return;
    }
    if (fgets(VideoId, sizeof(VideoId), fp) == NULL) {
        printf("CheckYoutube: No video ID\n");
        pclose(fp);
        return;
    }

    ExactTitle[strcspn(ExactTitle, "\n")] = 0;
    VideoId[strcspn(VideoId, "\n")] = 0;

    pclose(fp);

    printf("CheckYoutube: ExactTitle: %s, VideoId: %s\n", ExactTitle, VideoId);
    fflush(stdout);
}

char* DownloadAudioMp3(char* SongName, char* ArtistName) {
    printf("DownloadAudioMp3: START\n");
    fflush(stdout);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "yt-dlp \"ytsearch:%s %s\" --get-id --get-title --no-warnings 2>/dev/null",
        ArtistName, SongName);

    printf("DownloadAudioMp3: cmd: %s\n", cmd);
    fflush(stdout);

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        printf("Error with opening pipe in DownloadAudioMp3\n");
        exit(1);
    }

    char ExactTitle[256];
    char VideoId[64];

    fgets(ExactTitle, sizeof(ExactTitle), fp);
    fgets(VideoId, sizeof(VideoId), fp);

    ExactTitle[strcspn(ExactTitle, "\n")] = 0;
    VideoId[strcspn(VideoId, "\n")] = 0;

    printf("DownloadAudioMp3: VideoId: %s\n", VideoId);
    fflush(stdout);

    char* Mp3AudioPath = malloc(1024);
    if (!Mp3AudioPath) {
        pclose(fp);
        return NULL;
    }
    snprintf(Mp3AudioPath, 1024, "assets/audio/%s-%s.mp3", SongName, ArtistName);

    printf("DownloadAudioMp3: Mp3AudioPath: %s\n", Mp3AudioPath);
    fflush(stdout);

    snprintf(cmd, sizeof(cmd),
        "yt-dlp -f bestaudio --extract-audio --audio-format mp3 -o \"%s\" \"https://www.youtube.com/watch?v=%s\"",
        Mp3AudioPath, VideoId);
    printf("DownloadAudioMp3: download cmd: %s\n", cmd);
    fflush(stdout);

    int result = system(cmd);
    pclose(fp);

    if (result != 0) {
        printf("DownloadAudioMp3: Download failed\n");
        free(Mp3AudioPath);
        return NULL;
    }

    return Mp3AudioPath;
}

char* GetLyrics(char* exactArtist, char* exactTrack) {
    printf("GetLyrics: START\n");
    fflush(stdout);

    // URL-encode the exact names
    char* encodedArtist = curl_easy_escape(NULL, exactArtist, 0);
    char* encodedTrack = curl_easy_escape(NULL, exactTrack, 0);

    if (!encodedArtist || !encodedTrack) {
        if (encodedArtist) curl_free(encodedArtist);
        if (encodedTrack) curl_free(encodedTrack);
        return NULL;
    }

    char url[1024];
    snprintf(url, sizeof(url), "https://lrclib.net/api/get?artist_name=%s&track_name=%s", encodedArtist, encodedTrack);

    curl_free(encodedArtist);
    curl_free(encodedTrack);

    printf("GetLyrics: URL: %s\n", url);
    fflush(stdout);

    char* response_buffer = malloc(5242880);
    if (!response_buffer) return NULL;
    memset(response_buffer, 0, 5242880);

    CURL* curl = curl_easy_init();
    if (!curl) {
        free(response_buffer);
        return NULL;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    // Parse JSON and check syncedLyrics
    cJSON* json = cJSON_Parse(response_buffer);
    if (!json) {
        printf("GetLyrics: Failed to parse JSON\n");
        free(response_buffer);
        return NULL;
    }

    cJSON* synced = cJSON_GetObjectItem(json, "syncedLyrics");
    char* syncedStr = cJSON_GetStringValue(synced);
    if (syncedStr == NULL || strlen(syncedStr) == 0) {
        printf("GetLyrics: No synced lyrics found\n");
        cJSON_Delete(json);
        free(response_buffer);
        return NULL;
    }

    // Save to file
    char* JsonLyricsPath = malloc(1024);
    if (!JsonLyricsPath) {
        cJSON_Delete(json);
        free(response_buffer);
        return NULL;
    }
    snprintf(JsonLyricsPath, 1024, "assets/lyrics/%s-%s.json", exactArtist, exactTrack);

    printf("GetLyrics: JsonLyricsPath: %s\n", JsonLyricsPath);
    fflush(stdout);

    FILE* file = fopen(JsonLyricsPath, "w");
    if (file == NULL) {
        printf("Failed to create lyrics file\n");
        cJSON_Delete(json);
        free(response_buffer);
        free(JsonLyricsPath);
        return NULL;
    }

    // Write the full JSON
    fprintf(file, "%s", response_buffer);
    fclose(file);

    cJSON_Delete(json);
    free(response_buffer);
    return JsonLyricsPath;
}
