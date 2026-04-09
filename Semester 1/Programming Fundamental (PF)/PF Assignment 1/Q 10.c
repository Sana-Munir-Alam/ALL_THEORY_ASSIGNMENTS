#include <stdio.h>
#include <string.h>

// Function to correct the numbers typed by Mr. Bhoola
void CorrectNumber(char WrongNumber[], char CorrectedNumber[]) {
    int i = 0, j = 0;

    // Looping through each character of the wrong number string
    while (i < strlen(WrongNumber)) {
        // Check if the current and next characters are '9' and '0' respectively
        if (WrongNumber[i] == '9' && i + 1 < strlen(WrongNumber) && WrongNumber[i + 1] == '0') {
            // Writing '9' to corrected number and skipping the extra '0'
            CorrectedNumber[j++] = '9';
            i += 2;
        } else {
            CorrectedNumber[j++] = WrongNumber[i++];
        }
    }
    CorrectedNumber[j] = '\0'; // Null-terminate the corrected string
}

// Main function
int main() {
    char WrongNumber[100];     
    char CorrectedNumber[100];  
    
    printf("Enter the incorrect number typed by Mr. Bhoola: ");
    scanf("%s", WrongNumber);

    CorrectNumber(WrongNumber, CorrectedNumber);

    printf("The corrected number is: %s\n", CorrectedNumber);

    return 0;
}
