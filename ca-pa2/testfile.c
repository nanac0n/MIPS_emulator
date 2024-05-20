#include <stdio.h>

void main(){
    long long int a = 0b111110000000;
    long long int b = 0b000011111111;

    printf("this is shift right sra : %lld\n", a >>6);
    printf("this is shirt right srl : %lld\n", b >>6);
}