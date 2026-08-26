#include "functions.h"

int findCurrentLine(long int elapsed, int count, long int* milli) {

    // Goes through all the lines from last to first, and returns the line which the elapsed time has already surpassed
    // Making it return the exact line we should be at right now
    int i = count;
    while (elapsed < milli[i] && i>0){
        i--;
    }
    return i;
}
