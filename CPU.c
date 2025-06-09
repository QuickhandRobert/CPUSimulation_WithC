#include <stdio.h>
#include "headers.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
/*
IR:

The instruction register (IR) is used to hold the instruction that is currently being executed. The contents of IR are available to the control unit, which generate the timing signals that control the various processing elements involved in executing the instruction.

PC:

The Program Counter is one of the most important registers in the CPU. A program is a series of instructions stored in the memory. These instructions tell the CPU exactly how to get the desired result. It is important that these instructions must be executed in a proper order to get the correct result. This sequence of instruction execution is monitored by the program counter. It keeps track of which instruction is being executed and what the next instruction will be.

SP:

Stack pointer (SP) is used to point to the top activation record on the run-time stack. The run-time stack contains one activation record for each function or procedure invocation that is currently unfinished in the program. The top activation record corresponds to the current function invocation. When a function call is made an activation record is pushed onto the run-time stack. When a function returns, the activation record is popped by decrementing the stack pointer to point to the previous activation record.

AC:

AC is one of the general purpose registers but it is specifically used to 'accumulate' the result of the currently running instructions.

MAR:

MAR are used to handle the data transfer between the main memory and the processor. The MAR holds the address of the main memory to or from which data is to be transferred.

MDR:

MDR are used to handle the data transfer between the main memory and the processor. The MDR contains the data to be written into or read from the addressed word of the main memory.
*/
//CPU Registers
static char MDRStringHandler[STRING_SIZE];
static char R_IR[SYNTAX_LIMIT][STRING_SIZE];
static int R_PC; //Program Counter
static int R_SP; //Stack Pointer
static int R_AC; //Accumulator
static int R_MAR; //Memory Address
static int R_MDR[2]; //Value
static int R_X = 1; //General Purpose
static int R_Y = 0; //General Purpose
static int R_Z; //General Purpose
static char R_S[STRING_SIZE]; //String Handling
static char R_A[STRING_SIZE]; //String Handling
static char CONSTSTR[CONSTANT_STRINGS][STRING_SIZE];
//Debugging Purposes:

//Global Variables
int memoryAddress[2] = {0};
char errors[ERRORS][STRING_SIZE];

unsigned long fetch(char buffer[SYNTAX_LIMIT][STRING_SIZE]) {
	//Memory stuff shall be handled by a seperate function
	for (int i = 0; i < SYNTAX_LIMIT; i++) {
		readFromMemory(1, 0, R_PC, i, buffer[i]);
	}
	unsigned long a = readFromMemory(0, 0, R_PC, -1, "");
	R_PC++;
	return a;
}
void makeRegisterPointers(int n, registerP *registers) {
	for (int i = 0; i < n; i++) {
		switch(registers[i].hashed) {
			case SP:
				registers[i].p = &R_SP;
				registers[i].type = 0;
				break;
			case AC:
				registers[i].p = &R_AC;
				registers[i].type = 0;
				break;
			case MAR:
				registers[i].p = &R_MAR;
				registers[i].type = 0;
				break;
			case MDR:
				registers[i].p = &R_MDR[0];
				registers[i].type = 0;
				break;
			case X:
				registers[i].p = &R_X;
				registers[i].type = 0;
				break;
			case Y:
				registers[i].p = &R_Y;
				registers[i].type = 0;
				break;
			case Z:
				registers[i].p = &R_Z;
				registers[i].type = 0;
				break;
			case S:
				registers[i].p = R_S;
				registers[i].type = 1;
				break;
			case A:
				registers[i].p = R_A;
				registers[i].type = 1;
				break;
			default:
				registers[i].p = NULL;
				registers[i].type = -1;
				break;
		}
	}
}
//void makeStringRegisterPointers(int n, registerP *registers) {
//	for (int i = 0; i < n; i++) {
//		switch (registers[i].hashed) {
//			case S:
//				registers[i].p = R_S;
//				break;
//			case A:
//				registers[i].p = R_A;
//				break;
//			default:
//				registers[i].p = NULL;
//				break;
//
//		}
//	}
//}
void error_def() {
	char buffer[STRING_SIZE];
	char *res;
	int index;
	stringCut(buffer, *R_IR, 1, -1, ' ');
	index = atoi(buffer);
	strcpy(errors[index], R_IR[1]);
}
void print_error(registerP *registers) {
	char buffer[STRING_SIZE];
	char *res;
	int index, len;
	int i, j;
	stringCut(buffer, *R_IR, 1, -1, ' ');
	index = atoi(buffer);
	len=strlen(errors[index]);
	for (i = 0, j = 1; i < len; i++) {
		if (errors[index][i] == '#') {
			switch (registers[j].type){
			case 0:
				printf("%d", *(int *)registers[j].p);
				break;
			case 1:
				printf("%s", (char *)registers[j].p);
				break;
			}
			j++;
		}
		else {
			putchar(errors[index][i]);
		}
	}

}
void loadToMemory(FILE *drive, char *filename) {
	char buffer[LINE_BUFFER_SIZE];
	char *res;
	int i, j, k;
	unsigned long hashed = 0;
	if (!isStringEmpty(filename)) openFile(filename, drive);
	for (i = memoryAddress[1]; hashed != END; i++) {
		readLine(buffer, "", 0, drive);
		res = strtok_fixed(buffer, " ", '\"');
		hashed = hashStr(res);
		if (*res == '@') {
			i--;
			continue;
		}
		for (j = 0; hashed != END && res != NULL; j++) {
			if (!j) {
				writeToMemory(0, 0, i, -1, hashed, "");
			} else {
				writeToMemory(1, 0, i, j - 1, 0, res);
			}
			res = strtok_fixed(NULL, " ", '\"');
		}
	}
	memoryAddress[0] = memoryAddress[1];
	memoryAddress[1] = i - 1;

	//Memory stuff shall be handled by a seperate function
}
void decode_execute(unsigned long instruction, FILE *drive) {
	registerP registers[SYNTAX_LIMIT];
	char *stringRegisters[SYNTAX_LIMIT];
	char buffer2[STRING_SIZE];
	switch (instruction) {
		case OR:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_OR((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case AND:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_AND((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case NOT:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			F_NOT((int *)registers[0].p);
			break;
		case XOR:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_XOR((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case NAND:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_NAND((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case NOR:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_NOR((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case FOPEN:
			openFile(R_IR[0], drive);
			break;
		case MKF:
			createFile(R_IR[0], drive);
			break;
		case RL:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			readLine((char *)registers[0].p, R_IR[2], 1, drive);
			break;
		case RNC:
			hashRegisters(2, R_IR, registers);
			makeRegisterPointers(2, registers);
			*(char *)registers[1].p = readChar(R_IR[2], registers[0].p, drive);
			break;
		case PF:
			break;
		case FEX:
			break;
		case FSIZE:
			break;
		case ADD:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_ADD((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case SUB:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_SUB((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case DIV:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_DIV((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case MUL:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_MUL((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case LO:
			hashRegisters(3, R_IR, registers);
			makeRegisterPointers(3, registers);
			F_LO((int *)registers[0].p, (int *)registers[1].p, (int *)registers[2].p);
			break;
		case INC:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			F_INC((int *)registers[0].p);
			break;
		case DEC:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			F_DEC((int *)registers[0].p);
			break;
		case NEG:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			F_NEG((int *)registers[0].p);
			break;
		case COPY:
			break;
		case FLUSH:
			stringCut(buffer2, R_IR[0], 1, -1, "");
			flushMemoryAddress(atoi(buffer2), 1);
			break;
		case CLEAR:
			memoryInit(1);
			break;
		case MEMWRITE:
			writeToMemory(R_MDR[1], 1, R_MAR, -1, R_MDR[0], MDRStringHandler);
			break;
		case PMEMWRITE:
			writeToMemory(R_MDR[1], 1, R_SP, -1, R_MDR[0], MDRStringHandler);
			break;
		case MEMLOAD:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			*(int *)registers[0].p = readFromMemory(-1, 1, R_MAR, -1, (char *)registers[0].p);
			break;
		case PMEMLOAD:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			*(int *)registers[0].p = readFromMemory(-1, 1, R_SP, -1, (char *)registers[0].p);
			break;
		case REGSET:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			switch(registers[0].type){
				case 0:
					*(int *)registers[0].p = atoi(R_IR[1]);
					break;
				case 1:
					strcpy((char *)registers[0].p, R_IR[1]);
					break;
			}
			break;
		case MARSET:
			R_MAR = atoi(R_IR[0]);
			break;
		case REGCOPY:
			break;
		case EQ:
			break;
		case INPUT:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			switch(registers[0].type){
				case 0:
					*(int *)registers[0].p = read_int('\n');
					break;
				case 1:
					gets((char *)registers[0].p);
			}
			break;
		case OUTPUT:
			hashRegisters(1, R_IR, registers);
			makeRegisterPointers(1, registers);
			switch(registers[0].type){
				case 0:
					printf("%d", *(int *)registers[0].p);
					break;
				case 1:
					printf("%s", (char *)registers[0].p);
					break;
			}
			break;
		case CLS:
			system("cls");
			break;
		case GOTO:
			R_PC = atoi(R_IR[0]);
			break;
		case RUN:
			break;
		case CRUN:
			break;
		case SHUTDOWN:
			exit(0);
			break;
		case HIBERNATE:
			break;
		case CONST:
			break;
		case ERR:
			error_def();
			break;
		case CERR:
			hashRegisters(7, R_IR, registers);
			makeRegisterPointers(7, registers);
			print_error(registers);
			break;
		case RUNPROC:
			break;
	}

}
void runCPU(FILE *drive) {
	R_PC = 0;
	unsigned long instruction;
	//CPU Cycle
	while(R_PC < memoryAddress[1]) {
		instruction = fetch(R_IR);
		decode_execute(instruction, drive);
	}
}

