#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "../libs/cJSON/cJSON.h"
#include <unistd.h>
#include <sys/wait.h>

// Runs argv[0] with the given arguments directly via fork+execvp (no shell
// involved), and returns a FILE* streaming the child's stdout, like popen()
// but immune to shell-metacharacter injection from user-controlled strings.
// Caller must pclose()-style clean up with fclose() + waitpid on childPid.
static FILE* SpawnCaptureStdout(char* const argv[], pid_t* childPid) {
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        perror("pipe failed");
        return NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) {
        // Child: redirect stdout to the pipe, silence stderr, then exec
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        freopen("/dev/null", "w", stderr);
        execvp(argv[0], argv);
        perror("execvp failed");
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    FILE* fp = fdopen(pipefd[0], "r");
    if (!fp) {
        close(pipefd[0]);
        return NULL;
    }
    *childPid = pid;
    return fp;
}

// Replaces filesystem-unsafe characters ('/', '\', control chars, and a
// leading '.') with '_' so user-supplied song/artist names can't be used
// for path traversal (e.g. "../../etc") when building file paths, and can't
// break shell quoting when logged/printed.
static void SanitizeForFilename(const char* in, char* out, size_t outSize) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < outSize; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '/' || c == '\\' || c < 0x20 || c == 0x7f) {
            out[j++] = '_';
        } else {
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
    // Never allow the sanitized name to start with '.' (hides files / ".." tricks)
    if (out[0] == '.') out[0] = '_';
}

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

    char searchQuery[512];
    snprintf(searchQuery, sizeof(searchQuery), "ytsearch:%s %s", ArtistName, SongName);

    char* argv[] = { "yt-dlp", searchQuery, "--get-id", "--get-title", "--no-warnings", NULL };

    pid_t childPid;
    FILE* fp = SpawnCaptureStdout(argv, &childPid);
    if (!fp) {
        printf("Error with opening pipe in CheckYoutube\n");
        return;
    }

    char VideoId[64];
    char ExactTitle[256];

    if (fgets(ExactTitle, sizeof(ExactTitle), fp) == NULL) {
        printf("CheckYoutube: No exact title\n");
        fclose(fp);
        waitpid(childPid, NULL, 0);
        return;
    }
    if (fgets(VideoId, sizeof(VideoId), fp) == NULL) {
        printf("CheckYoutube: No video ID\n");
        fclose(fp);
        waitpid(childPid, NULL, 0);
        return;
    }

    ExactTitle[strcspn(ExactTitle, "\n")] = 0;
    VideoId[strcspn(VideoId, "\n")] = 0;

    fclose(fp);
    waitpid(childPid, NULL, 0);

    printf("CheckYoutube: ExactTitle: %s, VideoId: %s\n", ExactTitle, VideoId);
    fflush(stdout);
}

char* DownloadAudioMp3(char* SongName, char* ArtistName) {
    printf("DownloadAudioMp3: START\n");
    fflush(stdout);

    char searchQuery[512];
    snprintf(searchQuery, sizeof(searchQuery), "ytsearch:%s %s", ArtistName, SongName);
    char* searchArgv[] = { "yt-dlp", searchQuery, "--get-id", "--get-title", "--no-warnings", NULL };

    pid_t searchPid;
    FILE* fp = SpawnCaptureStdout(searchArgv, &searchPid);
    if (!fp) {
        printf("Error with opening pipe in DownloadAudioMp3\n");
        return NULL;
    }

    char ExactTitle[256];
    char VideoId[64];

    if (fgets(ExactTitle, sizeof(ExactTitle), fp) == NULL ||
        fgets(VideoId, sizeof(VideoId), fp) == NULL) {
        printf("DownloadAudioMp3: No search results\n");
        fclose(fp);
        waitpid(searchPid, NULL, 0);
        return NULL;
    }

    ExactTitle[strcspn(ExactTitle, "\n")] = 0;
    VideoId[strcspn(VideoId, "\n")] = 0;

    fclose(fp);
    waitpid(searchPid, NULL, 0);

    if (VideoId[0] == '\0') {
        printf("DownloadAudioMp3: Empty video ID\n");
        return NULL;
    }

    printf("DownloadAudioMp3: VideoId: %s\n", VideoId);
    fflush(stdout);

    // Sanitize names before using them in a filesystem path (no '/', '..', etc.)
    char safeSong[256], safeArtist[256];
    SanitizeForFilename(SongName, safeSong, sizeof(safeSong));
    SanitizeForFilename(ArtistName, safeArtist, sizeof(safeArtist));

    char* Mp3AudioPath = malloc(1024);
    if (!Mp3AudioPath) {
        return NULL;
    }
    snprintf(Mp3AudioPath, 1024, "assets/audio/%s-%s.mp3", safeSong, safeArtist);

    printf("DownloadAudioMp3: Mp3AudioPath: %s\n", Mp3AudioPath);
    fflush(stdout);

    char videoUrl[128];
    snprintf(videoUrl, sizeof(videoUrl), "https://www.youtube.com/watch?v=%s", VideoId);

    char* downloadArgv[] = {
        "yt-dlp", "-f", "bestaudio", "--extract-audio", "--audio-format", "mp3",
        "-o", Mp3AudioPath, videoUrl, NULL
    };

    pid_t downloadPid = fork();
    if (downloadPid < 0) {
        perror("fork failed");
        free(Mp3AudioPath);
        return NULL;
    }
    if (downloadPid == 0) {
        execvp(downloadArgv[0], downloadArgv);
        perror("execvp failed");
        _exit(127);
    }

    int status = 0;
    waitpid(downloadPid, &status, 0);

    if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
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
    char safeArtist[256], safeTrack[256];
    SanitizeForFilename(exactArtist, safeArtist, sizeof(safeArtist));
    SanitizeForFilename(exactTrack, safeTrack, sizeof(safeTrack));
    snprintf(JsonLyricsPath, 1024, "assets/lyrics/%s-%s.json", safeArtist, safeTrack);

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
