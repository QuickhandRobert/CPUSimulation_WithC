#include <stdio.h>
#include <string.h>
#include "headers.h"

//Function Declaration
//void clearMemory(void);
//void flushMemoryBlock(int);
//void writeToMemory(int, int, int, char*);
//void copyMemory(int, int);

//Global Variables
systemRAM systemMemory[MEMORY_SIZE];
userRAM userMemory [MEMORY_SIZE];
char stringMemory[MEMORY_SIZE][STRING_SIZE];
metadata metadataMemory[NUMBER_OF_FILES]; //Max Number of Files
int freeSectors[NUMBER_OF_FILES] = {0};
void memoryInit(int memoryType) {
	switch(memoryType) {
		case 0:
			for (int i = 0; i < MEMORY_SIZE; i++){
				systemMemory[i].isFree = 1;
				for (int j = 0; j < SYNTAX_LIMIT; j++)
					systemMemory[i].data[j][0] = '\0';
			} 
			break;
		case 1:
			for (int i = 0; i < MEMORY_SIZE; i++) userMemory[i].isFree = 1;
	}
}
void metadataMemoryInit() {
	for (int i = 0; i < NUMBER_OF_FILES; i++) metadataMemory[i].isFree = 1;
}
void flushMemoryAddress(int memoryBlockAddress, int memoryType) {
	switch(memoryType) {
		case 0:
			systemMemory[memoryBlockAddress].isFree = 1;
			break;
		case 1:
			userMemory[memoryBlockAddress].isFree = 1;
	}
}
void writeToMemory(int dataType, int memoryType, int memoryBlockAddress0, int memoryBlockAddress1, unsigned long long int number, char *str) {
	switch(memoryType) {
		case 0:
			if (memoryBlockAddress1 == -1)
				systemMemory[memoryBlockAddress0].instruction = number;
			else {
				removeNewLine(str);
				strcpy(systemMemory[memoryBlockAddress0].data[memoryBlockAddress1], str);
				systemMemory[memoryBlockAddress0].isFree = 0;
			}
			break;
		case 1:
			switch (dataType) {
				case 0:
					userMemory[memoryBlockAddress0].data = number;
					break;
				case 1:
					strcpy(stringMemory[memoryBlockAddress0], str);
					break;
			}
			userMemory[memoryBlockAddress0].type = dataType;
			userMemory[memoryBlockAddress0].isFree = 0;
			break;
	}
}
unsigned long readFromMemory(int dataType, int memoryType, int memoryBlockAddress0, int memoryBlockAddress1, char *buffer) {
	switch(memoryType) {
		case 0:
			if (memoryBlockAddress1 == -1) {
				return systemMemory[memoryBlockAddress0].instruction;
			} else {
				strcpy(buffer, systemMemory[memoryBlockAddress0].data[memoryBlockAddress1]);
			}
			break;
		case 1:
			if (userMemory[memoryBlockAddress0].type == 0) {
				return userMemory[memoryBlockAddress0].data;
			} else {
				strcpy(buffer, stringMemory[memoryBlockAddress0]);
			}
	}
}

