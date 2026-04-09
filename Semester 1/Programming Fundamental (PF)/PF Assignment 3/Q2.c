#include <stdio.h>
#include <string.h>

struct Player {
    char PlayerName[50];
    int BallScores[12];
    int TotalScore;
};

int ValidateScore(int Score) {
    return (Score >= 0 && Score <= 6);
}

void PlayGame(struct Player *Player1, struct Player *Player2) {
    for (int i = 0; i < 12; i++) {
        int Score;
        printf("\n");
        // Player 1's turn
        printf("Enter Score for ball %d (%s): ", i + 1, Player1->PlayerName);
        scanf("%d", &Score);
        if (ValidateScore(Score)) {
            Player1->BallScores[i] = Score;
            Player1->TotalScore += Score;
        }else {
            printf("Invalid Score! Ball marked but no Score added.\n");
            Player1->BallScores[i] = 0;
        }
        // Player 2's turn
        printf("Enter Score for ball %d (%s): ", i + 1, Player2->PlayerName);
        scanf("%d", &Score);
        if (ValidateScore(Score)) {
            Player2->BallScores[i] = Score;
            Player2->TotalScore += Score;
        }else {
            printf("Invalid Score! Ball marked but no Score added.\n");
            Player2->BallScores[i] = 0;
        }
    }
}

void DisplayScoreboard(struct Player Player1, struct Player Player2) {
    printf("\nMatch Scoreboard:\n");

    struct Player *players[] = {&Player1, &Player2};
    for (int i = 0; i < 2; i++) {
        struct Player *player = players[i];
        printf("\n%s's Performance:\n", player->PlayerName);
        printf("Ball-by-ball scores: ");
        for (int j = 0; j < 12; j++) {
            printf("%d ", player->BallScores[j]);
        }
        printf("\nTotal Score: %d\n", player->TotalScore);
        printf("Average Score: %.2f\n", player->TotalScore / 12.0);
    }
}

void FindWinner(struct Player Player1, struct Player Player2) {
    printf("\nMatch Result:\n");
    if (Player1.TotalScore > Player2.TotalScore) {
        printf("Winner: %s with %d runs!\n", Player1.PlayerName, Player1.TotalScore);
    } else if (Player2.TotalScore > Player1.TotalScore) {
        printf("Winner: %s with %d runs!\n", Player2.PlayerName, Player2.TotalScore);
    } else {
        printf("It's a tie! Both players Scored %d runs.\n", Player1.TotalScore);
    }
}

int main() {
    struct Player Player1, Player2;

    Player1.TotalScore = 0;
    Player2.TotalScore = 0;

    printf("Enter the name of Player 1: ");
    fgets(Player1.PlayerName, 30, stdin);
    Player1.PlayerName[strcspn(Player1.PlayerName, "\n")] = '\0'; 

    printf("Enter the name of Player 2: ");
    fgets(Player2.PlayerName, 30, stdin);
    Player2.PlayerName[strcspn(Player2.PlayerName, "\n")] = '\0'; 

    PlayGame(&Player1, &Player2);
    DisplayScoreboard(Player1, Player2);
    FindWinner(Player1, Player2);

    return 0;
}
