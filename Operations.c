#include <stdio.h>
#include "headers.h"
//
////Function Declration
//void F_OR(int, int, int);
//void F_AND(int, int, int);
//void F_NOT(int);
//void F_XOR(int, int, int);
//void F_ADD(int, int, int);
//void F_SUB(int, int, int);
//void F_DIV(int, int, int);
//void F_MULT(int, int, int);
//void F_LO(int, int, int);
//void F_INC(int);
//void F_DEC(int);
//void F_NEG(int);
//void F_CMP(int,int,int );
//void F_EQ(int, int, int);
//
////Global Variables
extern systemRAM systemMemory[MEMORY_SIZE];
extern userRAM userMemory [MEMORY_SIZE];

void F_OR(int *r1, int *r2, int *r3) {
	*r3 = *r1 || *r2;
}
void F_AND(int *r1, int *r2, int *r3) {
	*r3 = *r1 && *r2;
}
void F_NOT(int *r1) {
	*r1 = !*r1;
}
void F_XOR(int *r1, int *r2, int *r3) {
	*r3 = *r1 ^ *r2;
}
void F_NAND(int *r1, int *r2, int *r3) {
	*r3 = !(*r1 && *r2);
}
void F_NOR(int *r1, int *r2, int *r3) {
	*r3 = !(*r1 || *r2);
}
void F_ADD(int *r1, int *r2, int *r3) {
	*r3 = *r1 + *r2;
}
void F_SUB(int *r1, int *r2, int *r3) {
	*r3 = *r1 - *r2;
}
void F_DIV(int *r1, int *r2, int *r3) {
	*r3 = *r1 / *r2;
}
void F_MUL (int *r1, int *r2, int *r3) {
	*r3 = *r1 * *r2;
}
void F_LO(int *r1, int *r2, int *r3) {
	*r3 = *r1 % *r2;
}
void F_INC(int *r1) {
	*r1++;
}
void F_DEC(int *r1) {
	*r1--;
}
void F_NEG(int *r1) {
	*r1 *= -1;
}
//void F_CMP (int m1,int m2,int m3) {
//	if(userMemory[m1][0]>userMemory[m2][0]) {
//		userMemory[m3][0]=userMemory[m1][0];
//	} else if(userMemory[m2][0]>userMemory[m1][0]) {
//		userMemory[m3][0]=userMemory[m2][0];
//	}
//}
//void F_EQ(int m1, int m2, int m3) {
//	if(userMemory[m1][0] == userMemory[m2][0]) {
//		userMemory[m3][0] = 1;
//	}
//}

//
