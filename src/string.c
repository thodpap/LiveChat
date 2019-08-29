#include <stdio.h>
#include "string.h"

void clearArray(char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void clearOutput() {
    printf("\r%s", "> ");
    fflush(stdout);
}
