#include <stdio.h>
#include <stdlib.h>

struct Employee {
    int *Ratings;
    int TotalScore;
};

void InputEmployees(int** ratings, int NumEmployees, int NumPeriods) {
    for (int i = 0; i < NumEmployees; i++) {
        for (int j = 0; j < NumPeriods; j++) {
            int Rating;
            do {
                printf("Enter Rating for Employee %d, Period %d (Rating b/w 1 and 10): ", i + 1, j + 1);
                scanf("%d", &Rating);
                if (Rating < 1 || Rating > 10) {
                    printf("Invalid Rating! Kindly Enter a value between 1 and 10.\n");
                }
            } while (Rating < 1 || Rating > 10);
            ratings[i][j] = Rating;
        }
    }
}

void DisplayPerformance(int** Ratings, int NumEmployees, int NumPeriods) {
    for (int i = 0; i < NumEmployees; i++) {
        printf("\nPerformance of Employee %d:\n", i + 1);
        for (int j = 0; j < NumPeriods; j++) {
            printf("\tPeriod %d: %d\n", j + 1, Ratings[i][j]);
        }
    }
}

int FindEmployeeOfTheYear(int** Ratings, int NumEmployees, int NumPeriods) {
    int BestEmployee = 0;
    float HighestAvg = -1.0;
    for (int i = 0; i < NumEmployees; i++) {
        int Total = 0;
        for (int j = 0; j < NumPeriods; j++) {
            Total += Ratings[i][j];
        }
        float Avg = (float)Total / NumPeriods;
        if (Avg > HighestAvg) {
            HighestAvg = Avg;
            BestEmployee = i;
        }
    }
    return BestEmployee;
}

int FindHighestRatedPeriod(int** Ratings, int NumEmployees, int NumPeriods) {
    int BestPeriod = 0;
    float HighestAvg = -1.0;
    for (int i = 0; i < NumPeriods; i++) {
        int Total = 0;
        for (int j = 0; j < NumEmployees; j++) {
            Total += Ratings[j][i];
        }
        float Avg = (float)Total / NumEmployees;
        if (Avg > HighestAvg) {
            HighestAvg = Avg;
            BestPeriod = i;
        }
    }
    return BestPeriod;
}

int FindWorstPerformingEmployee(int** Ratings, int NumEmployees, int NumPeriods) {
    int WorstEmployee = 0;
    float LowestAvg = 11.0;
    for (int i = 0; i < NumEmployees; i++) {
        int Total = 0;
        for (int j = 0; j < NumPeriods; j++) {
            Total += Ratings[i][j];
        }
        float Avg = (float)Total / NumPeriods;
        if (Avg < LowestAvg) {
            LowestAvg = Avg;
            WorstEmployee = i;
        }
    }
    return WorstEmployee;
}

int main() {
    int NumEmployees;
    int NumPeriods;

    printf("Enter the Number of Employees: ");
    scanf("%d", &NumEmployees);
    printf("Enter the Number of Evaluation Periods: ");
    scanf("%d", &NumPeriods);

    int** ratings = (int**)malloc(NumEmployees * sizeof(int*));
    for (int i = 0; i < NumEmployees; i++) {
        ratings[i] = (int*)malloc(NumPeriods * sizeof(int));
    }

    InputEmployees(ratings, NumEmployees, NumPeriods);
    DisplayPerformance(ratings, NumEmployees, NumPeriods);

    int EmployeeOfTheYear = FindEmployeeOfTheYear(ratings, NumEmployees, NumPeriods);
    printf("\nEmployee of the Year: Employee %d\n", EmployeeOfTheYear + 1);

    int HighestRatedPeriod = FindHighestRatedPeriod(ratings, NumEmployees, NumPeriods);
    printf("\nHighest Rated Evaluation Period of the Year: Period %d\n", HighestRatedPeriod + 1);

    int WorstEmployee = FindWorstPerformingEmployee(ratings, NumEmployees, NumPeriods);
    printf("\nWorst Performing Employee: Employee %d\n\n", WorstEmployee + 1);

    for (int i = 0; i < NumEmployees; i++) {
        free(ratings[i]);
    }
    free(ratings);

    return 0;
}
