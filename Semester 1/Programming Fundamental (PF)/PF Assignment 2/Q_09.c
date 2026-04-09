#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
// Global Variables
int Score = 0;
int Px = 4, Py = 4; // Player's starting position

// Define grid globally so it can be modified by other functions
char Grid[5][5] = {
    {' ', ' ', 'I', 'X', ' '},
    {' ', 'X', ' ', ' ', ' '},
    {'I', ' ', 'X', 'X', ' '},
    {' ', ' ', ' ', 'I', 'X'},
    {' ', 'X', ' ', ' ', 'P'}
};

// Function to move the player based on input direction
void move(int move_x, int move_y) { 
    int newX = Px + move_x; 
    int newY = Py + move_y; 

    // Check if new position is within bounds
    if (newX >= 0 && newX < 5 && newY >= 0 && newY < 5) {
        // Check if new position is an obstacle
        if (Grid[newX][newY] != 'X') {
            // Collect item if present
            if (Grid[newX][newY] == 'I') {
                Score += 1;
            }
            // Update grid: clear old position and set new position
            Grid[Px][Py] = ' '; // Clear previous position
            Px = newX;
            Py = newY;
            Grid[Px][Py] = 'P'; // Update player position
        } else {
            printf("You hit an obstacle! Try a different move.\n");
        }
    } else {
        printf("Move out of bounds! Try again.\n");
    }
}
// Function to display the grid
void CreateMap() { 
	// Clear screen 
	system("cls");
    printf("\nWelcome To The Game!\n");
    printf("Instructions:\n-Press 'W' to move up.\n-Press 'A' to move left.\n-Press 'S' to move down.\n-Press 'D' to move right.\n-Press 'Q' to exit the game.\n\n");
	// Drawing All the elements in the screen 
	for (int i = 0; i < 5; i++) { 
		for (int j = 0; j < 5; j++) { 
			printf("%c", Grid[i][j]); 
		} 
		printf("\n"); 
	} 
	printf("Score: %d\n", Score); 
}

int main() {
    char Input;
    printf("\nWelcome To The Game!\n");
    printf("Instructions:\n-Press 'W' to move up.\n-Press 'A' to move left.\n-Press 'S' to move down.\n-Press 'D' to move right.\n-Press 'Q' to exit the game.\nPress \'Y\'to Start Game\n");
    Input = getch(); 
	if (Input != 'Y' && Input != 'y') { 
		printf("Exit Game! "); 
		return 1; 
	} 

    while (1) {
        CreateMap();
        printf("Enter your move: ");
        //scanf(" %c", &Input);
        Input = getch();
        // Move based on user input
        switch (Input) { 
            case 'W':
                move(-1, 0); // Move up
                break;
            case 'S':
                move(1, 0); // Move down
                break;
            case 'A':
                move(0, -1); // Move left
                break;
            case 'D':
                move(0, 1); // Move right
                break;
            case 'Q':
                printf("Game Over! Your Score: %d\n", Score);
                return 0;
            default:
                printf("Invalid move! Please enter W, A, S, D, or Q.\n");
                break;
        }
    }
    return 0;
}
