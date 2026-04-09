#include <stdio.h>
#include <string.h>

int main() {
    char LCDPattern[7]; 
    printf("Enter the 7-digit LCD pattern: ");
    scanf("%7s", LCDPattern);
    if (strcmp(LCDPattern, "1111110") == 0) {
        printf("0\n");
    } else if (strcmp(LCDPattern, "0110000") == 0) {
        printf("1\n");
    } else if (strcmp(LCDPattern, "1101101") == 0) {
        printf("2\n");
    } else if (strcmp(LCDPattern, "1111001") == 0) {
        printf("3\n");
    } else if (strcmp(LCDPattern, "0110011") == 0) {
        printf("4\n");
    } else if (strcmp(LCDPattern, "1011011") == 0) {
        printf("5\n");
    } else if (strcmp(LCDPattern, "1011111") == 0) {
        printf("6\n");
    } else if (strcmp(LCDPattern, "1110000") == 0) {
        printf("7\n");
    } else if (strcmp(LCDPattern, "1111111") == 0) {
        printf("8\n");
    } else if (strcmp(LCDPattern, "1111011") == 0) {
        printf("9\n");
    } else {
        printf("-1\n");
    }
    return 0;
}
