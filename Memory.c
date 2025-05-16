#include <stdio.h>
#include <string.h>
#include "systemConfiguration.h"

//Function Declaration
void clearMemory(void);
void flushMemoryBlock(int);
void writeToMemory(int, int, int[]);
void copyMemory(int, int);
//Structure Definitions
struct metadata {
	char filename[32];
	int size;
	int start_sector;
} ;
//Global Variables
int systemMemory [MEMORY_SIZE][10];
int userMemory [MEMORY_SIZE][STRING_SIZE];
struct metadata metadataMemory[DISK_SIZE / SECTOR_SIZE]; //Max Number of Files
void clearMemory(){
	for (int i = 0; i < MEMORY_SIZE; i++) for (int j = 0; j < STRING_SIZE; j++) userMemory[i][j] = EMPTY_MEMORY_VALUE;
}
void flushMemoryBlock(int memoryBlockAddress){
	for (int i = 0; i < STRING_SIZE; i++) userMemory [memoryBlockAddress][i] = EMPTY_MEMORY_VALUE;
}
void writeToMemory(int dataType, int memoryBlockAddress, int dataValue[]){
	switch (dataType){
		case 0:
			userMemory[memoryBlockAddress][0] = dataValue[0];
			break;
		case 1:
			stringCopy(dataValue, userMemory[memoryBlockAddress]);
			break;
	}
}
void copyMemory(int from, int to){
	for (int i = 0; userMemory[from][i] != 0x7FFFFFFF && userMemory[from][i] != '\0'; i++) userMemory[to][i] = userMemory[from][i];
}
