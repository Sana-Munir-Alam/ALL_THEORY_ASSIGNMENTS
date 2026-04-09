#include <stdio.h>
#include <string.h>

// Function to calculate frequency of characters in a slogan
void FrequencySlogan(const char *Slogan, int *Frequency) {
    int Length = strlen(Slogan);
    for (int i = 0; i < Length; i++) {
        int Count = 1;
        
        if (Frequency[i] != 0) {
            continue;
        }
        for (int j = i + 1; j < Length; j++) {
            if (Slogan[i] == Slogan[j]) {
                Count++;
                Frequency[j] = -1; //so that repeated characters are not checked again, and to avoid the characters printing again in output
            }
        }
        Frequency[i] = Count;
    }
}
// Function to iterate through slogan list
void SloganCounter(char Slogans[][100], int WordCount) {
    printf("\n");
    for (int i = 0; i < WordCount; i++) {
        int Frequency[100] = {0};  // Array to store frequency of characters for each slogan
        FrequencySlogan(Slogans[i], Frequency);

        printf("For Slogan \"%s\": {", Slogans[i]);
        for (int j = 0; Slogans[i][j] != '\0'; j++) {
            if (Frequency[j] > 0) {
                printf("'%c': %d, ", Slogans[i][j], Frequency[j]);
            }
        }
        printf("}\n");
    }
}

int main() {
    int n;
    printf("Enter the number of slogans: ");
    scanf("%d", &n);
    getchar();
    char Slogans[n][100];
    for (int i = 0; i < n; i++) {
        printf("Enter slogan[%d]: ", i + 1);
        fgets(Slogans[i], sizeof(Slogans[i]), stdin);
        size_t Length = strlen(Slogans[i]);
        if (Length > 0 && Slogans[i][Length - 1] == '\n') {
            Slogans[i][Length - 1] = '\0';
        }
    }
    SloganCounter(Slogans, n);
    return 0;
}
