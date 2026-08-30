#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include "../libs/cJSON/cJSON.h"
#define VOWEL_MS 180
#define CONSONANT_MS 20
#define MIN_WORD_MS 80
#define BASE_MS 30

int CalculateWordTimeValues(char* word, float bpm) {
    if (word == NULL) return 0;

    int vowels = 0;
    int consonants = 0;

    for (int i = 0; word[i] != '\0'; i++) {
        char c = tolower(word[i]);
        if (c >= 'a' && c <= 'z') {
            // Check for double vowels (oo, ee, aa, etc.) — count as one
            if (i > 0 && tolower(word[i]) == tolower(word[i-1])) {
                // Skip if it's a double letter
                continue;
            }


            if (i > 0) {
                char prev = tolower(word[i-1]);
                char curr = tolower(word[i]);
                if ((prev == 'o' && curr == 'u') ||
                    (prev == 'a' && curr == 'i') ||
                    (prev == 'e' && curr == 'a') ||
                    (prev == 'o' && curr == 'i') ||
                    (prev == 'e' && curr == 'i') ||
                    (prev == 'i' && curr == 'e') ||
                    (prev == 'u' && curr == 'i') ||
                    (prev == 'a' && curr == 'u')) {
                    continue;
                }
            }

            if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y') {
                vowels++;
            } else {
                consonants++;
            }
        }
    }

    float bpmFactor = 100.0 / bpm;
    int time = (int)((BASE_MS + vowels * VOWEL_MS + consonants * CONSONANT_MS) * bpmFactor);
    if (time < MIN_WORD_MS) time = MIN_WORD_MS;

    return time;
}




void ShrinkWordTimeValues(int* WordTimeValuesRow, int WordsPerLine, int LineTimeLength) {
    if (WordsPerLine <= 0) {
        printf("ERROR: WordsPerLine is 0 in ShrinkWordTimeValues\n");
        exit(1);
    }

    // Calculate sum
    int sum = 0;
    for (int i = 0; i < WordsPerLine; i++) {
        sum += WordTimeValuesRow[i];
    }

    // Shrink one by one until sum fits. Each word has a floor of 1ms, so if
    // the target is smaller than WordsPerLine, sum can never reach it — bail
    // out once a full pass makes no progress instead of looping forever.
    while (sum > LineTimeLength) {
        bool madeProgress = false;
        for (int i = 0; i < WordsPerLine && sum > LineTimeLength; i++) {
            if (WordTimeValuesRow[i] > 1) {
                WordTimeValuesRow[i]--;
                sum--;
                madeProgress = true;
            }
        }
        if (!madeProgress) break;
    }
}


void FillLineGap(int* WordTimeValuesRow, int WordsPerLine, int LineTimeLength) {
    if (WordsPerLine <= 0) {
        printf("ERROR: WordsPerLine is 0 in FillLineGap\n");
        exit(1);
    }

    // Calculate sum
    int sum = 0;
    for (int i = 0; i < WordsPerLine; i++) {
        sum += WordTimeValuesRow[i];
    }

    // Fill until sum reaches line length (but don't overshoot)
    while (sum < LineTimeLength) {
        // Add 1 to all words first, but don't overshoot
        for (int i = 0; i < WordsPerLine - 1 && sum < LineTimeLength; i++) {
            if (sum + 1 <= LineTimeLength) {
                WordTimeValuesRow[i]++;
                sum++;
            }
        }

        // Add 3 to the last word, but don't overshoot
        if (sum < LineTimeLength) {
            int add = 3;
            if (sum + add > LineTimeLength) {
                add = LineTimeLength - sum;
            }
            WordTimeValuesRow[WordsPerLine - 1] += add;
            sum += add;
        }
    }
}

int FindCurrentLineValue(long int CurrentTime, long int* LineResetTimeStamps, int TotalLinesHuman) {
    // Find which line we should be on based on the current time
    // Lines are 1-indexed (human numerals)
    // Returns 0 if before first line, TotalLinesHuman if after last

    if (CurrentTime < LineResetTimeStamps[0]) {
        return 0;  // Before first line
    }

    for (int i = 0; i < TotalLinesHuman - 1; i++) {
        if (CurrentTime >= LineResetTimeStamps[i] && CurrentTime < LineResetTimeStamps[i + 1]) {
            return i + 1;  // Human numeral (1-indexed)
        }
    }

    // After last line
    return TotalLinesHuman;
}

int FindCurrentWordValue(long int CurrentTime, int* WordTimeStamps, int TotalWordsHuman) {
    // Find which global word we should be on based on current time
    // Returns 1-indexed (human numerals)
    // Returns 0 if before first word, TotalWordsHuman if after last

    if (TotalWordsHuman == 0) {
        return 0;  // No words
    }

    if (CurrentTime < WordTimeStamps[0]) {
        return 0;  // Before first word
    }

    for (int i = 0; i < TotalWordsHuman - 1; i++) {
        if (CurrentTime >= WordTimeStamps[i] && CurrentTime < WordTimeStamps[i + 1]) {
            return i + 1;  // Human numeral (1-indexed)
        }
    }

    // After last word
    return TotalWordsHuman;
}

void UpdateBufferWords(char* Buffer, size_t BufferSize, char* Word) {
    if (Word == NULL || BufferSize == 0) {
        return;  // Do nothing, keep buffer as is
    }

    size_t used = strlen(Buffer);
    if (used + 1 < BufferSize) {
        strncat(Buffer, " ", BufferSize - used - 1);
        used = strlen(Buffer);
    }
    if (used < BufferSize - 1) {
        strncat(Buffer, Word, BufferSize - used - 1);
    }
}






long int* SeparateTimeStamps(char** lyrics, int TotalLinesHuman) {
    long int* timestamps = malloc(TotalLinesHuman * sizeof(long int));
    if (!timestamps) return NULL;

    for (int i = 0; i < TotalLinesHuman; i++) {
        char* line = lyrics[i];
        int minutes, seconds, centiseconds;

        // Parse [mm:ss.cc] at start of line
        if (sscanf(line, "[%d:%d.%d]", &minutes, &seconds, &centiseconds) == 3) {
            timestamps[i] = minutes * 60000 + seconds * 1000 + centiseconds * 10;
        } else {
            printf("No timestamp found in line: %s\n", line);
            exit(1); // No timestamp found
        }
    }

    return timestamps;
}


int* CalculateLineTimeLengths(long int* LineResetTimeStamps, int TotalLinesHuman, long int TotalTime) {
    int* lengths = malloc(TotalLinesHuman * sizeof(int));
    if (!lengths) return NULL;

    for (int i = 0; i < TotalLinesHuman - 1; i++) {
        lengths[i] = LineResetTimeStamps[i + 1] - LineResetTimeStamps[i];
        if (lengths[i] < 0) lengths[i] = 0;
    }
    // Last line runs until the song actually ends, not a hardcoded 0 —
    // a 0-length budget made ShrinkWordTimeValues loop forever on it.
    long int lastLen = TotalTime - LineResetTimeStamps[TotalLinesHuman - 1];
    lengths[TotalLinesHuman - 1] = (lastLen > 0) ? (int)lastLen : 0;

    return lengths;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int CheckAubioExists() {
    // Check if aubio is installed
    if (system("which aubio > /dev/null 2>&1") == 0) {
        return 1;  // Exists
    }
    return 0;  // Not found
}

float GetBpmFromAubio(char* mp3Path) {
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return 120.0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return 120.0;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        freopen("/dev/null", "w", stderr);
        char* argv[] = { "aubio", "tempo", mp3Path, NULL };
        execvp(argv[0], argv);
        _exit(127);
    }

    close(pipefd[1]);
    FILE* fp = fdopen(pipefd[0], "r");
    if (!fp) {
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return 120.0;
    }

    char output[256];
    float bpm = 120.0;

    while (fgets(output, sizeof(output), fp)) {
        // aubio outputs: "tempo: 120.00 bpm"
        if (strstr(output, "tempo:") != NULL) {
            sscanf(output, "tempo: %f", &bpm);
            break;
        }
    }

    fclose(fp);
    waitpid(pid, NULL, 0);
    return bpm;
}
