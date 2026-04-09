#include <stdio.h>
//Function to calculate whether Player A will win or not
int Result(int NumMatch){
    int GameResult;
    GameResult = NumMatch % 5;
    if(GameResult == 0){
        return -1;
    }else{
        return GameResult;
    }
}

int main(){
    int NumMatch;
    printf("Enter the number of match sticks: ");
    scanf("%d", &NumMatch);
    int Pick = Result(NumMatch);
    if (Pick == -1){
        printf("%d (Can't win)", Pick);
    }else{
        printf("Player A can win by picking %d matchsticks,", Pick);
    }
    return 0;
}
