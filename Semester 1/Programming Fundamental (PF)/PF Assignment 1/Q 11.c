#include <stdio.h>

int main() {
    int Num1, Num2, GCD;

    printf("Enter the first number: ");
    scanf("%d", &Num1);

    printf("Enter the second number: ");
    scanf("%d", &Num2);

    if (Num1 > 0 && Num2 > 0) {
        // Finding the GCD using the Euclidean algorithm
        int a = Num1, b = Num2;
        while (b != 0) {
            int Temp = b;
            b = a % b;
            a = Temp;
        }
        GCD = a;

        if (GCD == 1) {
            printf("The numbers %d and %d are coprime.\n", Num1, Num2);
        } else {
            printf("The numbers %d and %d are not coprime.\n", Num1, Num2);
        }
    } else {
        printf("Invalid input! Both numbers must be greater than 0.\n");
    }

    return 0;
}
