#include <stdio.h>
#include <string.h>
#include "headers.h"

//Global Variables
systemRAM systemMemory[MEMORY_SIZE];
userRAM userMemory [MEMORY_SIZE];
//Error Handler
extern enum errors_def error_code;
extern char error_buff[STRING_SIZE];
/******************************************************
* Func: memAddress_check                              *
* Params: const int memoryAddress:                    *
*                                                     *
* Return: Input address exists or not                 *
******************************************************/
bool memAddress_check(const int memoryAddress) {
	if (memoryAddress > MEMORY_SIZE) {
		error_code = MEMORY_LIMIT_EXCEEDED;
		sprintf(error_buff, "%d", memoryAddress);
		p_error(false);
		return false;
	} else
		return true;
}
/******************************************************
* Func: memoryInit                                    *
* Params: int memoryType: self-explanatory            *
*                                                     *
* Return: none                                        *
* Desc: Frees every memoryslot of a given memorytype  *
******************************************************/
void memoryInit(int memoryType) {
	switch(memoryType) {
		case SYSTEM_RAM:
			for (int i = 0; i < MEMORY_SIZE; i++)
				systemMemory[i].isFree = true;
			break;
		case USER_RAM:
			for (int i = 0; i < MEMORY_SIZE; i++)
				userMemory[i].isFree = true;
	}
}
/************************************************************
* Func: flushMemoryAddress                                  *
* Params: const int memoryBlockAddress                      *
*         const int memoryType (Both self-explanatory)      *
*                                                           *
* Return: none                                              *
************************************************************/
void flushMemoryAddress(const int memoryBlockAddress, const int memoryType) {
	if (!memAddress_check(memoryBlockAddress)) //Sanity check
		return;
	switch(memoryType) {
		case SYSTEM_RAM:
			systemMemory[memoryBlockAddress].isFree = true;
			break;
		case USER_RAM:
			userMemory[memoryBlockAddress].isFree = true;
	}
}
/*********************************************************************************
* Func: writeToMemory                                                            *
* Params: const int dataType: M_INT or M_STR                                     *
*         const int memoryType: SYSTEM_RAM or USER_RAM                           *
*         const index0: Memory Address                                           *
*         const index1: Parameter index (SYSTEM_RAM only)                        *
*         const unsigned long number: Hashed value of the current instruction    *
*         char *str: String Output                                               *
*                                                                                *
*         Return: none                                                           *
*********************************************************************************/
void writeToMemory(const int dataType, const int memoryType, const int index0, const int index1, const unsigned long number, char *str) {
	if (!memAddress_check(index0)) //Sanity check
		return;
	switch(memoryType) {
		case SYSTEM_RAM:
			if (index1 == -1)
				systemMemory[index0].instruction = number;
			else {
				removeNewLine(str);
				strcpy(systemMemory[index0].data[index1], str);
				systemMemory[index0].isFree = false;
			}
			break;
		case USER_RAM:
			userMemory[index0].data = (int)number;
			userMemory[index0].isFree = false;
			break;
	}
}
/*********************************************************************************
* Func: readFromMemory                                                           *
* Params: const int dataType: M_INT or M_STR                                     *
*         const int memoryType: SYSTEM_RAM or USER_RAM                           *
*         const index0: Memory Address                                           *
*         const index1: Parameter index (SYSTEM_RAM only)                        *
*         char *buffer: String Output                                            *
*                                                                                *
*         Return: Read value                                                     *
*********************************************************************************/
unsigned long readFromMemory(const int dataType, const int memoryType, const int memoryBlockAddress0, const int memoryBlockAddress1, char *buffer) {
	if (!memAddress_check(memoryBlockAddress0))
		return 1;
	switch(memoryType) {
		case SYSTEM_RAM:
			if (memoryBlockAddress1 == -1) {
				return systemMemory[memoryBlockAddress0].instruction;
			} else {
				strcpy(buffer, systemMemory[memoryBlockAddress0].data[memoryBlockAddress1]);
			}
			break;
		case USER_RAM:
			return userMemory[memoryBlockAddress0].data;
	}
}
bool mem_isFree(const int memoryType, const int index) {
	if (!memAddress_check(index)) //Sanity check
		return false;
	switch (memoryType) {
		case SYSTEM_RAM:
			return systemMemory[index].isFree;
			break;
		case USER_RAM:
			return userMemory[index].isFree;
			break;
	}
}

