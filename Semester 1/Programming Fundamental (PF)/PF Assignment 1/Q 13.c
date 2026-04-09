#include <stdio.h>

int main() {
    int M, N, X;
    int JugM = 0, JugN = 0;
    
    printf("Enter the capacity of the first jug (M liters): ");
    scanf("%d", &M);
    printf("Enter the capacity of the second jug (N liters): ");
    scanf("%d", &N);
    printf("Enter the amount of water you want to measure (X liters): ");
    scanf("%d", &X);
    
    if (N > M) {
        int temp = M;
        M = N;
        N = temp;
    }
    
    int a = M;
    int b = N;
     while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    int gcdMN = a;

    printf("You can measure the following multiples of GCD(%d): ", gcdMN);
    for (int i = 1; i * gcdMN <= M; i++) {
        printf("%d ", i * gcdMN);
    }
    printf("\n");

    if (X % gcdMN != 0 || X > M) {
        printf("It is not possible to measure exactly %d liters with the given jugs.\n", X);
        return 0;
    }

    while (JugM != X && JugN != X) {
        if (JugM == 0) {
            JugM = M;
            printf("Fill the %d-liter jug. JugM: %d liters, JugN: %d liters\n", M, JugM, JugN);
        } else if (JugN == N) {
            JugN = 0;
            printf("Empty the %d-liter jug. JugM: %d liters, JugN: %d liters\n", N, JugM, JugN);
        } else {
            int pourAmount = (JugM < (N - JugN)) ? JugM : (N - JugN);
            JugM -= pourAmount;
            JugN += pourAmount;
            printf("Pour from %d-liter jug to %d-liter jug. JugM: %d liters, JugN: %d liters\n", M, N, JugM, JugN);
        }
    }
    printf("Measured exactly %d liters of water!\n", X);
    return 0;
}
