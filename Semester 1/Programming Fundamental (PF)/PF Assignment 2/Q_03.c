#include <stdio.h>
#include<string.h>
// Function for Single Word Compression
void compressWord(const char *Words, char *Output, int *RemovedChars) {
    int Count = 0;
    int j = 0;
    for (int i = 0; Words[i] != '\0'; i++) { //this will ietrtate through the words character one by one
        if (i == 0 || Words[i] != Words[i - 1]) {
            Output[j] = Words[i];
            j = j +1;
        } else {
            Count = Count + 1;
        }
    }
    Output[j] = '\0';
    *RemovedChars = *RemovedChars + Count;
}

// Function for Multiple Word Compression
void compressWords(char Words[][100], int WordCount, char Output[][100], int *TotalRemovedChars){
    *TotalRemovedChars = 0;
    for (int i = 0; i < WordCount; i++) { //this will ietreate through the words list
        compressWord(Words[i], Output[i], TotalRemovedChars);
    }
}

int main() {
    int n;
    printf("Enter the number of words you wish to compress: ");
    scanf("%d", &n);
    getchar();
    char Words[n][100]; // [n]:stores the list of words, [100]max space to stores the length of each word
    for (int i = 0; i < n; i++) {
        printf("Enter word[%d]: ", i + 1);
        fgets(Words[i], sizeof(Words[i]), stdin); // Read word from user
        // Remove newline character if fgets adds it
        size_t Length = strlen(Words[i]);
        if (Length > 0 && Words[i][Length - 1] == '\n') {
            Words[i][Length - 1] = '\0';
        }
    }
    char Output[n][100];
    int TotalRemovedChars = 0;
    compressWords(Words, n, Output, &TotalRemovedChars); //array, counter, output_variable, remove_counter
    printf("\n");
    printf("Compressed Words: [");
    for (int i = 0; i < n; i++) {
        printf("\"%s\"", Output[i]);
        if (i != n - 1) {
            printf(", ");
        }
    }
    printf("]\n");
    printf("Total Characters Removed: %d\n", TotalRemovedChars);

    return 0;
}
