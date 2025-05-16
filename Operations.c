#include <stdio.h>
#include "systemConfiguration.h"

//Function Declration
void OR(int, int, int);
void AND(int, int, int);
void NOT(int);
void XOR(int, int, int);

//Global Variables
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];

void OR(int m1, int m2, int m3){
	userMemory[m3][0] = userMemory[m1][0] || userMemory[m2][0];
} 
void AND(int m1, int m2, int m3){
	userMemory[m3][0] = userMemory[m1][0] && userMemory[m2][0];
} 
void NOT(int m1){
	userMemory[m1][0] = !userMemory[m1][0];
} 
void XOR(int m1, int m2, int m3){
	userMemory[m3][0] = userMemory[m1][0] ^ userMemory[m2][0];
} 


