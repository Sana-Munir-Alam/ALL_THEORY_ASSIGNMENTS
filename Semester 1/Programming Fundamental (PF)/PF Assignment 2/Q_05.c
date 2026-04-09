#include <stdio.h>

// Function of Horizontal Histogram
void HorizontalHistogram(int Histogram[], int Count){
    printf("Horizontal Histogram:\n");
    for(int i = 0; i < Count; i++){
        printf("Value %d: ", i+1);
        for (int j = 0; j < Histogram[i]; j++){
            printf("*");
        }
        printf("\n");
    }
        
}
// Function of Vertical Histogram
void VerticalHistogram(int Histogram[], int Count){
    int Max = 0;
    //finding max value in the 
    for (int i = 0; i < Count; i++){
        if (Histogram[i] > Max){
            Max = Histogram[i];
        }
    }
    printf("\nVertical Histogram:\n");
    for (int i = Max; i > 0; i--){
        for (int j = 0; j < Count; j++){
            if (Histogram[j] >= i){
                printf("*");
            }else{
                printf(" ");
            }
        }
        printf("\n");
    }
    for(int i = 0; i < Count; i++){
        printf("%d", Histogram[i]);
    }
}

int main(){
    int Count;
    printf("Enter the number of values to define histogram range: ");
    scanf("%d", &Count);
    int Histogram[Count];
    for(int i = 0; i < Count; i++){
        printf("Enter the Value of Array[%d]: ", i);
        scanf("%d", &Histogram[i]);
    }
    printf("\n");
    //Call for Horizontal Histogram
    HorizontalHistogram(Histogram, Count);
    //Call for Vertical Histogram
    VerticalHistogram(Histogram, Count);

    return 0;
}
