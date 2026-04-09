#include <stdio.h>

int main() {
    int Number, Sum, Temp;

    printf("Enter a number: ");
    scanf("%d", &Number);
    Sum = 0;
    Temp = Number;

    while (Temp != 0) {
        Sum = Sum + (Temp % 10);
        Temp = Temp / 10;
    }
  
    printf("The sum of digits of number is %d\n", Sum);

    return 0;
}
