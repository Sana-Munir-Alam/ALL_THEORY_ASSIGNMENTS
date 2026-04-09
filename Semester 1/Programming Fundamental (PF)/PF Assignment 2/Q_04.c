#include <stdio.h>
#include <string.h>

//Function to compare the words in the array if they are anagram or not
int AreAnagram(char *List1, char *List2){
    int Count[26] = {0}; // Array to store frequency of each character 26 because this is the quantity of english alphabet
    for(int i = 0; List1[i] != '\0'; i++){
        Count[List1[i] - 'a'] = Count[List1[i] - 'a'] + 1;  //Increment counter for each alphabet encountered
    }
    for(int i = 0; List2[i] != '\0'; i++){
        Count[List2[i] - 'a'] = Count[List2[i] - 'a'] - 1;  //Decrement counter for each alphabet encountered
    }

    // Check if all frequencies are zero
    for (int i = 0; i < 26; i++){
        if (Count[i] != 0){ //if the count array is not zero it means that not all same alphabets were existing in those 2 words, hence are not anagrams
            return 0;
        }
    }
    return 1; // it means that alphabets were same in both the words hence are anagrams
}

// Function to traverse across the array to check words
void GroupAnagrams(char List[][100], int Num){
    int Grouped[100] = {0}; //using 100 with an assumption that entered words won't be greater than 100 (as i am unable to initialise it if i use int Grouped[Num] ok hence the use of 100)
    for (int i = 0; i < Num; i++){
        if (Grouped[i]){
            continue;
        }
        printf("[");
        printf("\'%s\'", List[i]);
        Grouped[i] = 1; // Mark that word as grouped
        
        for (int j = i+1; j < Num; j++){
            if (!Grouped[j] && AreAnagram(List[i], List[j])){
                printf(", \'%s\'", List[j]);
                Grouped[j] = 1; //Mark that word as grouped
            }
        }
        printf("],");
    }
    printf("]");
}

int main(){
    int Num;
    printf("\nNote(Enter words in small letter only)\n");
    printf("Enter the number of words you want to enter: ");
    scanf("%d", &Num);
    char List[Num][100];
    getchar();
    
    for (int i = 0; i < Num; i++) {
        printf("Enter Word[%d]: ", i);
        fgets(List[i], sizeof(List[i]), stdin); // Read word from user
        // Remove newline character if fgets adds it
        size_t Length = strlen(List[i]);
        if (Length > 0 && List[i][Length - 1] == '\n') {
            List[i][Length - 1] = '\0';
        }
    }

    printf("\n\nGrouped Anagrams:\n");
    GroupAnagrams(List, Num);
    return 0;
}
