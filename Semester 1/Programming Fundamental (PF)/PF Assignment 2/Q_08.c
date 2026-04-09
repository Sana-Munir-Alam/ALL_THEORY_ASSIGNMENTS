#include <stdio.h>
#include <string.h>

int BinaryToDecimal() {
    char Num[100];
    int a;
    int Base = 1;
    int DecimalValue = 0;
    printf("Enter Binary Number: ");
    scanf("%s", Num);
    // Validation
    for (int i = 0; i < strlen(Num); i++) {
        if (Num[i] != '0' && Num[i] != '1') {
            printf("Invalid binary input!\n");
            return -1;
        }
    }
    for (int i = strlen(Num) - 1; i >= 0; i--) {
        a = Num[i] - '0';  // Convert char to integer
        DecimalValue = DecimalValue + (a * Base);
        Base = Base * 2;
    }
    return DecimalValue;
}

int DecimalToBinary() {
    int Num, a;
    int Factor = 1;
    int BinaryValue = 0;

    printf("Enter Decimal Number (positive integer only): ");
    scanf("%d", &Num);
    //Validation
    if (Num < 0) {
        printf("Invalid decimal input!\n");
        return -1;
    }
    while (Num > 0) {
        a = Num % 2;
        Num = Num / 2;
        BinaryValue = BinaryValue + (a * Factor);
        Factor = Factor * 10; //Helps move into the bits position of binary from right to left.
    }

    return BinaryValue;
}


void DecimalToHexadecimal() {
    int Num, a;
    char Hex[20];
    int index = 0;
    printf("Enter Decimal Number: ");
    scanf("%d", &Num);
    //Validation
    if (Num < 0) {
        printf("Invalid decimal input!\n");
        return;
    }
    if (Num == 0) {
        printf("Hexadecimal Value: 0\n");
        return;
    }
    while (Num > 0) {
        a = Num % 16;
        Num = Num / 16;
        if (a < 10) {
            Hex[index++] = a + '0'; // Convert 0-9 to character
        }else {
            Hex[index++] = a - 10 + 'A'; // Convert 10-15 to 'A'-'F'
        }
    }
    printf("Hexadecimal Value: ");
    for (int i = index - 1; i >= 0; i--) {
        printf("%c", Hex[i]);
    }
    printf("\n");
}

void HexadecimalToDecimal(){
    char HexNum[20], a;
    int Base = 1;
    int DecimalValue = 0;
    printf("Enter Hexadecimal Value (Caps Value Only): ");
    scanf("%s", HexNum);
    //Validation
    for (int i = 0; i < strlen(HexNum); i++) {
        if (!((HexNum[i] >= '0' && HexNum[i] <= '9') || (HexNum[i] >= 'A' && HexNum[i] <= 'F'))) {
            printf("Invalid hexadecimal input!\n");
            return;
        }
    }
    int Length = strlen(HexNum);
    for (int i = Length - 1; i >= 0; i--) {
        char CurrentChar = HexNum[i];
        int CurrentValue;
        if (CurrentChar >= '0' && CurrentChar <= '9') {
            CurrentValue = CurrentChar - '0'; // Convert '0'-'9' to 0-9
        }else if (CurrentChar >= 'A' && CurrentChar <= 'F') {
            CurrentValue = CurrentChar - 'A' + 10; // Convert 'A'-'F' to 10-15
        }else {
            printf("Invalid hexadecimal input.\n");
            return;
        }
        DecimalValue = DecimalValue + (CurrentValue * Base);
        Base = Base * 16;
    }
    printf("Decimal Value: %d\n", DecimalValue);
}

void BinaryToHexadecimal() {
    char Num[100];
    char Hexa[20];
    int Length, HexIndex = 0;
    printf("Enter Binary Number: ");
    scanf("%s", Num);
    //Validation
    for (int i = 0; i < strlen(Num); i++) {
        if (Num[i] != '0' && Num[i] != '1') {
            printf("Invalid binary input!\n");
            return;
        }
    }
    Length = strlen(Num);
    // Add zeroes to extreme left in order to make it into grp of 4 (as conversion is done through 8 4 2 1|8 4 2 1)
    int a = Length % 4;
    if (a != 0) {
        for (int i = Length - 1; i >= 0; i--) {
            Num[i + (4 - a)] = Num[i];
        }
        for (int i = 0; i < 4 - a; i++) {
            Num[i] = '0';
        }
        Length = Length + (4 - a);
    }
    Num[Length] = '\0';

    for (int i = 0; i < Length; i += 4) {
        int value = 0;
        // Convert group of 4 binary digits to decimal
        if (Num[i] == '1'){
            value = value + 8;
        }if (Num[i + 1] == '1'){
            value = value + 4;
        }if (Num[i + 2] == '1'){
            value = value + 2;
        }if (Num[i + 3] == '1'){
            value = value + 1;
        }

        if (value < 10) {
            Hexa[HexIndex++] = value + '0';  // Convert to char '0' to '9'
        } else {
            Hexa[HexIndex++] = (value - 10) + 'A';  // Convert to char 'A' to 'F'
        }
    }
    Hexa[HexIndex] = '\0';
    printf("Hexadecimal Value: %s\n", Hexa);
}

void HexadecimalToBinary() {
    char Hexa[20];
    printf("Enter Hexadecimal Value: ");
    scanf("%s", Hexa);
    //Validation
    for (int i = 0; i < strlen(Hexa); i++) {
        if (!((Hexa[i] >= '0' && Hexa[i] <= '9') || (Hexa[i] >= 'A' && Hexa[i] <= 'F'))) {
            printf("Invalid hexadecimal input!\n");
            return;
        }
    }

    size_t i = (Hexa[1] == 'x' || Hexa[1] == 'X') ? 2 : 0;
	printf("Binary Value: ");
    while (Hexa[i]) {
        switch (Hexa[i]) {
            case '0':
                printf("0000");
                break;
            case '1':
                printf("0001");
                break;
            case '2':
                printf("0010");
                break;
            case '3':
                printf("0011");
                break;
            case '4':
                printf("0100");
                break;
            case '5':
                printf("0101");
                break;
            case '6':
                printf("0110");
                break;
            case '7':
                printf("0111");
                break;
            case '8':
                printf("1000");
                break;
            case '9':
                printf("1001");
                break;
            case 'A':
                printf("1010");
                break;
            case 'B':
                printf("1011");
                break;
            case 'C':
                printf("1100");
                break;
            case 'D':
                printf("1101");
                break;
            case 'E':
                printf("1110");
                break;
            case 'F':
                printf("1111");
                break;
            default:
                printf("\nInvalid hexadecimal digit: %c\n", Hexa[i]);
                return;
        }
        i++;
    }
    printf("\n");
}
int main() {
    int Choice;

    while (1) {
        printf("\nConvertor Menu: \n");
        printf("1. Binary to Decimal\n");
        printf("2. Decimal to Binary\n");
        printf("3. Decimal to Hexa-Decimal\n");
        printf("4. Hexa-decimal to Decimal\n");
        printf("5. Binary to Hexa-Decimal\n");
        printf("6. Hexa-Decimal to Binary\n");
        printf("7. Exit\n");
        printf("Kindly enter your choice: ");
        scanf("%d", &Choice);
        
        switch (Choice) {
            case 1:
                printf("Decimal Value: %d\n", BinaryToDecimal());
                break;
            case 2:
                printf("Binary Value: %d\n", DecimalToBinary());
                break;
            case 3:
                DecimalToHexadecimal();
                break;
            case 4:
                HexadecimalToDecimal();
                break;
            case 5:
                BinaryToHexadecimal();
                break;
            case 6:
                HexadecimalToBinary();
                break;
            case 7:
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}
