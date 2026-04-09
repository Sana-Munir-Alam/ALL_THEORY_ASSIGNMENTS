#include <stdio.h>

int main() {
    int Jug5 = 0, Jug3 = 0;
    int Temp;

    printf("Step 1: Fill the 5-liter jug completely.\n");
    Jug5 = 5;
    printf("\tJug5: %d liters\n", Jug5);
    printf("\tJug3: %d liters\n", Jug3);

    printf("Step 2: Pour water from the 5-liter jug into the 3-liter jug until the 3-liter jug is full or the 5-liter jug is empty.\n");
    Temp = Jug3;
    Jug3 = (Jug3 + Jug5 > 3) ? 3 : Jug3 + Jug5;  // Jug3 can hold a maximum of 3 liters
    Jug5 = Jug5 - (Jug3 - Temp);  // Reduce Jug5 by the amount poured into Jug3
    printf("\tJug5: %d liters\n", Jug5);
    printf("\tJug3: %d liters\n", Jug3);
    
    printf("Step 3: Empty the 3-liter jug.\n");
    Jug3 = 0;
    printf("\tJug5: %d liters\n", Jug5);
    printf("\tJug3: %d liters\n", Jug3);

    printf("Step 4: Pour the remaining water from the 5-liter jug into the 3-liter jug.\n");
    Temp = Jug3;
    Jug3 = (Jug3 + Jug5 > 3) ? 3 : Jug3 + Jug5;
    Jug5 = Jug5 - (Jug3 - Temp);
    printf("\tJug5: %d liters\n", Jug5);
    printf("\tJug3: %d liters\n", Jug3);

    printf("Step 5: Fill the 5-liter jug completely again.\n");
    Jug5 = 5;
    printf("\tJug5: %d liters\n", Jug5);
    printf("\tJug3: %d liters\n", Jug3);

    printf("Step 6: Pour water from the 5-liter jug into the 3-liter jug until the 3-liter jug is full or the 5-liter jug is empty.\n");
    Temp = Jug3;
    Jug3 = (Jug3 + Jug5 > 3) ? 3 : Jug3 + Jug5;
    Jug5 = Jug5 - (Jug3 - Temp);
    printf("\tJug5: %d liters\n", Jug5);
    printf("\tJug3: %d liters\n", Jug3);

    printf("The 5-liter jug now holds 4 liters of water.\n");

    return 0;
}
