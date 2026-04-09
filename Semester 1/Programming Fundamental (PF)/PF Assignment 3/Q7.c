#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants for content metadata
#define NUM_CATEGORIES 3
#define NUM_CONTENTS 3
#define NUM_DEVICES 2

// Content Metadata Structure
struct Content {
    char title[50];
    float rating;
    int runtime; // in minutes
};

// Device Preference Structure
struct DevicePreferences {
    int resolution; // e.g., 1080 for 1080p
    float bandwidthUsage; // in Mbps
};

// Function to initialize content metadata (fixed data)
void initializeContentMetadata(struct Content contentMatrix[NUM_CATEGORIES][NUM_CONTENTS]) {
    char titles[NUM_CATEGORIES][NUM_CONTENTS][50] = {
        {"Action1", "Action2", "Action3"},
        {"Drama1", "Drama2", "Drama3"},
        {"Comedy1", "Comedy2", "Comedy3"}
    };
    float ratings[NUM_CATEGORIES][NUM_CONTENTS] = {
        {4.5, 4.0, 3.8},
        {4.7, 4.2, 4.1},
        {3.9, 3.8, 3.6}
    };
    int runtimes[NUM_CATEGORIES][NUM_CONTENTS] = {
        {120, 150, 100},
        {140, 160, 130},
        {110, 90, 95}
    };

    for (int i = 0; i < NUM_CATEGORIES; i++) {
        for (int j = 0; j < NUM_CONTENTS; j++) {
            strcpy(contentMatrix[i][j].title, titles[i][j]);
            contentMatrix[i][j].rating = ratings[i][j];
            contentMatrix[i][j].runtime = runtimes[i][j];
        }
    }
}

// Function to input device preferences for a user
void inputDevicePreferences(struct DevicePreferences **deviceMatrix, int userIndex) {
    for (int i = 0; i < NUM_DEVICES; i++) {
        printf("Enter resolution for device %d (e.g., 1080): ", i + 1);
        scanf("%d", &deviceMatrix[userIndex][i].resolution);
        printf("Enter bandwidth usage for device %d (Mbps): ", i + 1);
        scanf("%f", &deviceMatrix[userIndex][i].bandwidthUsage);
    }
}

// Function to input engagement scores for a user
void inputEngagementScores(double **engagementMatrix, int userIndex) {
    printf("Enter engagement scores for user %d (0-100) for each category:\n", userIndex + 1);
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        printf("Category %d: ", i + 1);
        scanf("%lf", &engagementMatrix[userIndex][i]);
    }
}

// Function to display the content metadata
void displayContentMetadata(struct Content contentMatrix[NUM_CATEGORIES][NUM_CONTENTS]) {
    printf("\n--- Content Metadata ---\n");
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        printf("\nCategory %d:\n", i + 1);
        for (int j = 0; j < NUM_CONTENTS; j++) {
            printf("Title: %s | Rating: %.1f | Runtime: %d mins\n",
                   contentMatrix[i][j].title,
                   contentMatrix[i][j].rating,
                   contentMatrix[i][j].runtime);
        }
    }
}

// Function to display device preferences
void displayDevicePreferences(struct DevicePreferences **deviceMatrix, int userIndex) {
    printf("\n--- Device Preferences for User %d ---\n", userIndex + 1);
    for (int i = 0; i < NUM_DEVICES; i++) {
        printf("Device %d | Resolution: %dp | Bandwidth Usage: %.2f Mbps\n",
               i + 1,
               deviceMatrix[userIndex][i].resolution,
               deviceMatrix[userIndex][i].bandwidthUsage);
    }
}

// Function to display engagement scores
void displayEngagementScores(double **engagementMatrix, int userIndex) {
    printf("\n--- Engagement Scores for User %d ---\n", userIndex + 1);
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        printf("Category %d: %.2f%%\n", i + 1, engagementMatrix[userIndex][i]);
    }
}

int main() {
    int numUsers;

    printf("Enter the number of users: ");
    scanf("%d", &numUsers);

    // Allocate memory for device preferences and engagement matrix
    struct DevicePreferences **deviceMatrix = (struct DevicePreferences **)malloc(numUsers * sizeof(struct DevicePreferences *));
    for (int i = 0; i < numUsers; i++) {
        deviceMatrix[i] = (struct DevicePreferences *)malloc(NUM_DEVICES * sizeof(struct DevicePreferences));
    }

    double **engagementMatrix = (double **)malloc(numUsers * sizeof(double *));
    for (int i = 0; i < numUsers; i++) {
        engagementMatrix[i] = (double *)malloc(NUM_CATEGORIES * sizeof(double));
    }

    // Initialize and display content metadata
    struct Content contentMatrix[NUM_CATEGORIES][NUM_CONTENTS];
    initializeContentMetadata(contentMatrix);
    displayContentMetadata(contentMatrix);

    // Input and display data for each user
    for (int i = 0; i < numUsers; i++) {
        printf("\n--- Input Data for User %d ---\n", i + 1);
        inputDevicePreferences(deviceMatrix, i);
        inputEngagementScores(engagementMatrix, i);

        displayDevicePreferences(deviceMatrix, i);
        displayEngagementScores(engagementMatrix, i);
    }

    // Free allocated memory
    for (int i = 0; i < numUsers; i++) {
        free(deviceMatrix[i]);
        free(engagementMatrix[i]);
    }
    free(deviceMatrix);
    free(engagementMatrix);

    return 0;
}
