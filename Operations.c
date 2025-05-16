#include <stdio.h>
#include "systemConfiguration.h"

//Function Declration
void OR(int, int, int);
void AND(int, int, int);
void NOT(int);
void XOR(int, int, int);
void ADD(int, int, int);
void SUB(int, int, int);
void DIV(int, int, int);
void MULT(int, int, int);
void LO(int, int, int);
void INC(int);
void DEC(int);
void NEG(int);
void CMP(int ,int ,int );
void EQ(int , int , int);

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
void ADD(int m1, int m2, int m3){
	userMemory[m3][0]=userMemory[m1][0]+userMemory[m2][0];
}
void SUB(int m1, int m2, int m3){
	userMemory[m3][0]=userMemory[m1][0]-userMemory[m2][0];
}
void DIV(int m1, int m2, int m3){
	if(userMemory[m2][0]!= 0){
		userMemory[m3][0]=userMemory[m1][0]/userMemory[m2][0];
	}
}
void MULT(int m1, int m2, int m3){
	userMemory[m3][0]=userMemory[m1][0]*userMemory[m2][0];
}
void LO(int m1 , int m2 , int m3){
	userMemory[m3][0] = userMemory[m1][0] % userMemory[m2][0];
}
void INC(int m1){
	userMemory[m1][0] = userMemory[m1][0]++;
}
void DEC(int m1){
	userMemory[m1][0] = userMemory[m1][0]--;
}
void NEG(int m1){
	userMemory[m1][0] = -userMemory[m1][0];
}
void CMP (int m1,int m2,int m3){
	if(userMemory[m1][0]>userMemory[m2][0]){
		userMemory[m3][0]=userMemory[m1][0];
	}
	else if(userMemory[m2][0]>userMemory[m1][0]){
		userMemory[m3][0]=userMemory[m2][0];
	}
}
void EQ(int m1, int m2, int m3){
	if(userMemory[m1][0] == userMemory[m2][0]){
		userMemory[m3][0] = 1;
	}
}


