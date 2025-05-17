#include <stdio.h>
#include "systemConfiguration.h"

//Function Declration
void F_OR(int, int, int);
void F_AND(int, int, int);
void F_NOT(int);
void F_XOR(int, int, int);
void F_ADD(int, int, int);
void F_SUB(int, int, int);
void F_DIV(int, int, int);
void F_MULT(int, int, int);
void F_LO(int, int, int);
void F_INC(int);
void F_DEC(int);
void F_NEG(int);
void F_CMP(int,int,int );
void F_EQ(int, int, int);

//Global Variables
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];

void F_OR(int m1, int m2, int m3) {
	userMemory[m3][0] = userMemory[m1][0] || userMemory[m2][0];
}
void F_AND(int m1, int m2, int m3) {
	userMemory[m3][0] = userMemory[m1][0] && userMemory[m2][0];
}
void F_NOT(int m1) {
	userMemory[m1][0] = !userMemory[m1][0];
}
void F_XOR(int m1, int m2, int m3) {
	userMemory[m3][0] = userMemory[m1][0] ^ userMemory[m2][0];
}
void F_ADD(int m1, int m2, int m3) {
	userMemory[m3][0]=userMemory[m1][0]+userMemory[m2][0];
}
void F_SUB(int m1, int m2, int m3) {
	userMemory[m3][0]=userMemory[m1][0]-userMemory[m2][0];
}
void F_DIV(int m1, int m2, int m3) {
	if(userMemory[m2][0]!= 0) {
		userMemory[m3][0]=userMemory[m1][0]/userMemory[m2][0];
	}
}
void F_MULT(int m1, int m2, int m3) {
	userMemory[m3][0]=userMemory[m1][0]*userMemory[m2][0];
}
void F_LO(int m1, int m2, int m3) {
	userMemory[m3][0] = userMemory[m1][0] % userMemory[m2][0];
}
void F_INC(int m1) {
	userMemory[m1][0] = userMemory[m1][0]++;
}
void F_DEC(int m1) {
	userMemory[m1][0] = userMemory[m1][0]--;
}
void F_NEG(int m1) {
	userMemory[m1][0] = -userMemory[m1][0];
}
void F_CMP (int m1,int m2,int m3) {
	if(userMemory[m1][0]>userMemory[m2][0]) {
		userMemory[m3][0]=userMemory[m1][0];
	} else if(userMemory[m2][0]>userMemory[m1][0]) {
		userMemory[m3][0]=userMemory[m2][0];
	}
}
void F_EQ(int m1, int m2, int m3) {
	if(userMemory[m1][0] == userMemory[m2][0]) {
		userMemory[m3][0] = 1;
	}
}


