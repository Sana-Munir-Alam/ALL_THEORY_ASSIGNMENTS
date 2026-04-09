#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initializeInventory(char ***speciesSupplies, char **speciesNames, int *numSupplies, int numSpecies) {
    for (int i = 0; i < numSpecies; i++) {
        speciesNames[i] = (char *)malloc(50 * sizeof(char));
        printf("Enter Name For Species %d: ", i + 1);
        scanf("%s", speciesNames[i]);
        speciesSupplies[i] = (char **)malloc(100 * sizeof(char *)); 
        numSupplies[i] = 0;
    }
}
void addSupplies(char **speciesSupplies, int *numSupplies) {
    int numToAdd;
    printf("Enter the number of supplies to add: ");
    scanf("%d", &numToAdd);

    for (int i = 0; i < numToAdd; i++) {
        if (*numSupplies >= 100) {
            printf("Maximum supplies reached for this species.\n");
            break;
        }
        speciesSupplies[*numSupplies] = (char *)malloc(50 * sizeof(char));
        printf("Enter supply %d: ", *numSupplies + 1);
        scanf("%s", speciesSupplies[*numSupplies]);
        (*numSupplies)++;
    }
}
void updateSupply(char **speciesSupplies, int numSupplies) {
    int index;
    printf("Enter the supply index to update (1 to %d): ", numSupplies);
    scanf("%d", &index);
    if (index < 1 || index > numSupplies) {
        printf("Invalid supply index.\n");
        return;
    }
    printf("Enter new name for supply %d: ", index);
    scanf("%s", speciesSupplies[index - 1]);
}
void removeSpecies(char ***speciesSupplies, char **speciesNames, int *numSupplies, int *numSpecies) {
    int index;
    printf("Enter the index of the species to remove (0 to %d): ", *numSpecies - 1);
    scanf("%d", &index);
    if (index < 0 || index >= *numSpecies) {
        printf("Invalid index.\n");
        return;
    }

    for (int j = 0; j < numSupplies[index]; j++) {
        free(speciesSupplies[index][j]);
    }
    free(speciesSupplies[index]);
    free(speciesNames[index]);

    for (int i = index; i < *numSpecies - 1; i++) {
        speciesSupplies[i] = speciesSupplies[i + 1];
        speciesNames[i] = speciesNames[i + 1];
        numSupplies[i] = numSupplies[i + 1];
    }
    (*numSpecies)--;
    printf("Species removed successfully.\n");
}
void displayInventory(char ***speciesSupplies, char **speciesNames, int *numSupplies, int numSpecies) {
    printf("\n--- Current Inventory ---\n");
    for (int i = 0; i < numSpecies; i++) {
        printf("Species: %s\n", speciesNames[i]);
        for (int j = 0; j < numSupplies[i]; j++) {
            printf("  Supply %d: %s\n", j + 1, speciesSupplies[i][j]);
        }
    }
}

int main() {
    char **speciesSupplies[100];
    char *speciesNames[100];
    int numSupplies[100] = {0};
    int numSpecies;

    printf("Enter the number of species: ");
    scanf("%d", &numSpecies);
    printf("\n");\
    initializeInventory(speciesSupplies, speciesNames, numSupplies, numSpecies);

    int choice;
    while (1) {
        printf("\n--- Pets in Heart Inventory System ---\n");
        printf("1. Add Supplies for a Species\n");
        printf("2. Update a Supply for a Species\n");
        printf("3. Remove a Species\n");
        printf("4. Display Inventory\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: {
                if (numSpecies > 0) {
                    int speciesIndex;
                    printf("Enter species index (0 to %d): ", numSpecies - 1);
                    scanf("%d", &speciesIndex);
                    if (speciesIndex >= 0 && speciesIndex < numSpecies) {
                        addSupplies(speciesSupplies[speciesIndex], &numSupplies[speciesIndex]);
                    } else {
                        printf("Invalid species index.\n");
                    }
                } else {
                    printf("No species available.\n");
                }
                break;
            }
            case 2: {
                if (numSpecies > 0) {
                    int speciesIndex;
                    printf("Enter species index (0 to %d): ", numSpecies - 1);
                    scanf("%d", &speciesIndex);
                    if (speciesIndex >= 0 && speciesIndex < numSpecies) {
                        updateSupply(speciesSupplies[speciesIndex], numSupplies[speciesIndex]);
                    } else {
                        printf("Invalid species index.\n");
                    }
                } else {
                    printf("No species available.\n");
                }
                break;
            }
            case 3:
                removeSpecies(speciesSupplies, speciesNames, numSupplies, &numSpecies);
                break;
            case 4:
                displayInventory(speciesSupplies, speciesNames, numSupplies, numSpecies);
                break;
            case 5:
                for (int i = 0; i < numSpecies; i++) {
                    for (int j = 0; j < numSupplies[i]; j++) {
                        free(speciesSupplies[i][j]);
                    }
                    free(speciesSupplies[i]);
                    free(speciesNames[i]);
                }
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}
