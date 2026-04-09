#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 6
#define COLS 5

//Function to create a 6x5 array and inserting random characters
void CreateArray(char Array[ROWS][COLS], char LastDigits[]) {
    //Fill the first 5 rows with random characters
    for (int i = 0; i < ROWS - 1; i++) {
        for (int j = 0; j < COLS; j++) {
            Array[i][j] = 'A' + (rand() % 26); // Random uppercase letters
        }
    }
    // Add the last row with the last four digits and a random character
    for (int j = 0; j < COLS; j++) {
        if (j < 4) {
            Array[ROWS - 1][j] = LastDigits[j];
        } else {
            Array[ROWS - 1][j] = 'A' + (rand() % 26);
        }
    }
}

// Function to print the array in a tabular form
void PrintArray(char Array[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%c ", Array[i][j]);
        }
        printf("\n");
    }
}

// Function to search for a string in the 2D array (Horizontally and Vertically)
int SearchString(char Array[ROWS][COLS], char *str) {
    int len = strlen(str);
    // Search for the string horizontally in each row
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j <= COLS - len; j++) {
            int found = 1;
            for (int k = 0; k < len; k++) {
                if (Array[i][j + k] != str[k]) {
                    found = 0;
                    break;
                }
            }
            if (found) {
                return 1; // String found horizontally
            }
        }
    }
    // Search for the string vertically in each column
    for (int j = 0; j < COLS; j++) {
        for (int i = 0; i <= ROWS - len; i++) {
            int found = 1;
            for (int k = 0; k < len; k++) {
                if (Array[i + k][j] != str[k]) {
                    found = 0;
                    break;
                }
            }
            if (found) {
                return 1; // String found vertically
            }
        }
    }
    return 0; // String not found
}

int main() {
    char Array[ROWS][COLS];
    char LastDigits[5] = "0573"; // Replace the last row with Roll Number
    int Score = 0;
    char Input[100];

    // Create and display the initial array
    CreateArray(Array, LastDigits);
    PrintArray(Array);

    while (1) {
        printf("Enter a string to search (or type 'END' to stop): ");
        scanf("%s", Input);

        if (strcmp(Input, "END") == 0) { // When user wants to exit the program
            break;
        }
        // Search the input string in the array
        if (SearchString(Array, Input)) {
            Score = Score + 1;
            printf("%s is present. Score: %d\n", Input, Score);
        } else {
            Score = Score - 1;
            printf("%s is not present. Score: %d\n", Input, Score);
        }
    }
    printf("Final score: %d\n", Score);
    return 0;
}
