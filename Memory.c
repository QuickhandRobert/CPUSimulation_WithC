#include <stdio.h>
#include "systemConfiguration.h"
int systemMemory [MEMORY_SIZE][STRING_SIZE];

//Function Declaration
void clearMemory(void);
void flushMemoryBlock(int);



void clearMemory(){
	for (int i = 0; i < MEMORY_SIZE; i++) for (int j = 0; j < STRING_SIZE; j++) systemMemory[i][j] = 0x7FFFFFFF;
}
void flushMemoryBlock(int memoryBlock){
	for (int i = 0; i < STRING_SIZE; i++) systemMemory [memoryBlock][i] = 0x7FFFFFFF;
}
