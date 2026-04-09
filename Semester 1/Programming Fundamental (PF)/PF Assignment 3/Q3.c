#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int ValidateEmail(char* Email){
    int SymbolCount = 0;
    int DotAfterSymbol = 0;
    if (Email == NULL || strlen(Email) == 0){
        return 0;
    }
    for (char *ptr = Email; *ptr != '\0'; ptr++){
        if (*ptr == ' '){
            return 0;
        }else if (*ptr == '@'){
            SymbolCount++;
            if (SymbolCount != 1){
                return 0;
            }
        }else if (SymbolCount == 1 && *ptr == '.'){
            DotAfterSymbol = 1;
        }
    }
    if(SymbolCount == 1 && DotAfterSymbol == 1){
        return 1;
    }
    return 0;   
}

int main(){
    char *Email;
    Email = (char *)malloc(100 * sizeof(char));
    if(!Email){
        printf("Memory Allocation Failed");
        return 1;
    }
    
    printf("Enter the E-mail address: ");
    if (fgets(Email, 100, stdin) == NULL){
        printf("Input Not Read");
        free(Email);
        return 1;
    }
    Email[strcspn(Email, "\n")] = '\0';

    if (ValidateEmail(Email)){
        printf("Valid Email\n");
    }else {
        printf("Invalid Email\n");
    }
    free(Email);
    return 0;
}
