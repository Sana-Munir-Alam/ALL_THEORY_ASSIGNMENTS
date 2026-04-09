#include <stdio.h>
#include <stdbool.h>

// Function to check if a year is a leap year
bool IsLeapYear(int Year) {
    if ((Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0)) {
        return true;
    } else {
        return false;
    }
}

// Function to get the number of days in a month
int DaysInMonth(int Month, int Year) {
    switch (Month) {
        case 2:
            return IsLeapYear(Year) ? 29 : 28;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        default:
            return 31;
    }
}

// Function to validate if a date is valid
bool IsValidDate(int Day, int Month, int Year) {
    if (Month < 1 || Month > 12) {
        return false;
    } else if (Day < 1 || Day > DaysInMonth(Month, Year)) {
        return false;
    } else {
        return true;
    }
}

// Function to calculate the number of days between two dates
int DaysBetweenDates(int StartDay, int StartMonth, int StartYear, int EndDay, int EndMonth, int EndYear) {
    int TotalDays = 0;

    // Adding days from the start date to the end of the start year.
    for (int Month = StartMonth; Month <= 12; Month++) {
        if (Month == StartMonth) {
            TotalDays += DaysInMonth(Month, StartYear) - StartDay + 1;
        } else {
            TotalDays += DaysInMonth(Month, StartYear);
        }
    }

    // Adding days for the full years in between birth year and current year.
    for (int Year = StartYear + 1; Year < EndYear; Year++) {
        TotalDays += IsLeapYear(Year) ? 366 : 365;
    }

    // Adding days from the start of the end year to the end date.
    for (int Month = 1; Month < EndMonth; Month++) {
        TotalDays += DaysInMonth(Month, EndYear);
    }
    TotalDays += EndDay;

    return TotalDays;
}

// Main function
int main() {
    int BirthDay, BirthMonth, BirthYear;
    int CurrentDay, CurrentMonth, CurrentYear;

    printf("Enter the person's birthdate (day month year): ");
    scanf("%d %d %d", &BirthDay, &BirthMonth, &BirthYear);

    printf("Enter today's date (day month year): ");
    scanf("%d %d %d", &CurrentDay, &CurrentMonth, &CurrentYear);

    // Validating the input dates.
    if (!IsValidDate(BirthDay, BirthMonth, BirthYear) || !IsValidDate(CurrentDay, CurrentMonth, CurrentYear)) {
        printf("Invalid date entered.\n");
        return 1;
    }

    // Calculate total number of days between the dates.
    int TotalDays = DaysBetweenDates(BirthDay, BirthMonth, BirthYear, CurrentDay, CurrentMonth, CurrentYear);

    // Initialize year, month, and day counters
    int AgeYears = 0, AgeMonths = 0, AgeDays = 0;
    int TempMonth = BirthMonth;

    // Calculate age in years
    while (TotalDays >= 365) {
        if (IsLeapYear(BirthYear + AgeYears) && TotalDays >= 366) {
            TotalDays -= 366;
        } else {
            TotalDays -= 365;
        }
        AgeYears++;
    }

    // Calculate age in months
    while (TotalDays >= DaysInMonth(TempMonth, BirthYear + AgeYears)) {
        TotalDays -= DaysInMonth(TempMonth, BirthYear + AgeYears);
        AgeMonths++;
        TempMonth++;
        if (TempMonth > 12) {
            TempMonth = 1;
        }
    }

    // Remaining days
    AgeDays = TotalDays;

    // Print the result
    printf("Age: %d Years, %d Months, %d Days\n", AgeYears, AgeMonths, AgeDays);
    
    return 0;
}
