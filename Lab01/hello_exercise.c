#include<stdio.h>

int main(){
    FILE *fptr;
    char c;
    int LETTER_COUNT = 0, DIGIT_COUNT = 0;
    if((fptr = fopen("MyTextFile.txt","r")) == NULL){
        printf("Error in reading\n");
        return -1;
    }else{
        while((c = fgetc(fptr)) != EOF){
            if(c >= 48 && c <= 57){
                DIGIT_COUNT++;
            }
            else{
                if((c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
                    LETTER_COUNT++;
                }
            }
        }
        printf("The number of letters in this file is: %d\n", LETTER_COUNT);
        printf("The number of digits in this file is: %d\n", DIGIT_COUNT);
    }
    fclose(fptr);
    return 0;
}