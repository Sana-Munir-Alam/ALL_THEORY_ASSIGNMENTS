#include <stdio.h>
#include <string.h>
#include <time.h>

struct Employee{
    int EmployeeCode;
    char EmployeeName[50];
    char DateOfJoining [11];
};

void AssignEmployeeDetails(struct Employee *Info){
    printf("Enter Employee Code: ");
    scanf("%d", &Info->EmployeeCode);

    printf("Enter Employee Name: ");
    getchar();
    fgets(Info->EmployeeName, sizeof(Info->EmployeeName), stdin);
    Info->EmployeeName[strcspn(Info->EmployeeName, "\n")] = '\0';

    printf("Enter Date of Joining (DD/MM/YYYY): ");
    scanf("%s", Info->DateOfJoining);
}

int CheckTenure(struct Employee *Info, const char *CurrentDate){
    int Day, Month, Year;
    int DOJ_Day, DOJ_Month, DOJ_Year;
    int Tenure;
    sscanf(CurrentDate, "%d/%d/%d", &Day,&Month,&Year);
    sscanf(Info->DateOfJoining, "%d/%d/%d", &DOJ_Day, &DOJ_Month, &DOJ_Year);
    
    Tenure = Year - DOJ_Year;
    if (Month < DOJ_Month || Day < DOJ_Day){
        Tenure--;
    }
    return Tenure > 3;
}

int main(){
    struct Employee Info[4];
    int CountThreeYearPlus = 0;
    char CurrentDate[11];
    
    // Employeee Input
    for (int i = 0; i < 4; i++){
        printf("\nEnter Details for Employee %d\n", i+1);
        AssignEmployeeDetails(&Info[i]);
    }

    // Current Date Input
    printf("\nEnter Current Date (DD/MM/YYYY): ");
    scanf("%s", CurrentDate);


    //Check Tenure
    printf("\nEmployees with tenure more than 3 years:\n");
    for (int i = 0; i < 4; i++){
        if (CheckTenure(&Info[i], CurrentDate)){
            printf("\nEmployee Code: %d\n", Info[i].EmployeeCode);
            printf("Employee Name: %s\n", Info[i].EmployeeName);
            printf("Date of Joining: %s\n", Info[i].DateOfJoining);
            CountThreeYearPlus++;
        }
    }
    printf("\nTotal Employees with more than 3 years of tenure: %d\n", CountThreeYearPlus);

    return 0;
}