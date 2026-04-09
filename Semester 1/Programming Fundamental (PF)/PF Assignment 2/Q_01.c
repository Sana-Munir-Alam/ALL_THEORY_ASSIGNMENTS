#include<stdio.h>

int main(){
    int Array[5];
    int Min;
    int SecondMin;
    for(int i = 0; i < 5; i++){
        printf("Element[%d]: ", i);
        scanf("%d", &Array[i]);
    }
    Min = Array[0];
    SecondMin = Array[4];
    for(int i = 0; i < 5; i++){
        if(Array[i] < Min){
            Min = Array[i];
        }
    }
    for(int i = 0; i < 5; i++){
        if(Array[i] < SecondMin && Array[i] > Min){
            SecondMin = Array[i];
        }
    }
    printf("The Second Smallest Element in the Array is: %d", SecondMin);
    return 0;
}
