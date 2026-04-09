#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main() {
    float Height = 0.0; // Float variable for height
    int Age = 0; // Integer variable for age
    char RideType[50]; // String variable for ride
    bool ContinueRide = true; // Boolean variable to control loop

    printf("Enter your height in inches: ");
    scanf("%f", &Height);
    printf("Enter your age: ");
    scanf("%d", &Age);

    while (ContinueRide) {
        printf("Enter the ride you want (The Dragon Roller Coaster, The Sky Swing, The Carousel): ");
        scanf(" %[^\n]%*c", RideType); // Read string with spaces

        if (strcmp(RideType, "The Dragon Roller Coaster") == 0) {
            if (Height >= 48 && Age >= 10) {
                printf("You meet the criteria for The Dragon Roller Coaster.\n");
            } else {
                printf("Sorry, you do not meet the criteria for The Dragon Roller Coaster.\n");
            }
        } else if (strcmp(RideType, "The Sky Swing") == 0) {
            if (Height >= 54) {
                printf("You meet the criteria for The Sky Swing.\n");
            } else {
                printf("Sorry, you do not meet the criteria for The Sky Swing.\n");
            }
        } else if (strcmp(RideType, "The Carousel") == 0) {
            if (Age >= 5) {
                printf("You meet the criteria for The Carousel.\n");
            } else {
                printf("Sorry, you do not meet the criteria for The Carousel.\n");
            }
        } else {
            printf("Invalid ride selection.\n");
        }
        
        char Response[4]; //
        printf("Do you want to select another ride? (yes/no): ");
        scanf("%s",Response);

        if (strcmp(Response, "no") == 0) {
            ContinueRide = false;
        }
    }

    return 0;
}
