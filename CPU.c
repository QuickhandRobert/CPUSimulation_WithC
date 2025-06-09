#include <stdio.h>
#include "headers.h"
#include <string.h>
#include <ctype.h>

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

unsigned long fetch(char buffer[SYNTAX_LIMIT][STRING_SIZE]) {
	//Memory stuff shall be handled by a seperate function
	for (int i = 0; i < SYNTAX_LIMIT; i++) {
		readFromMemory(1, 0, R_PC, i, buffer[i]);
	}
	unsigned long a = readFromMemory(0, 0, R_PC, -1, "");
	R_PC++;
	return a;
}
void makeRegisterPointers(int n, unsigned long int *hashed, int **reg) {
	for (int i = 0; i < n; i++) {
		switch(hashed[i]) {
			case SP:
				reg[i] = &R_SP;
				break;
			case AC:
				reg[i] = &R_AC;
				break;
			case MAR:
				reg[i] = &R_MAR;
				break;
			case MDR:
				reg[i] = &R_MDR[0];
				break;
			case X:
				reg[i] = &R_X;
				break;
			case Y:
				reg[i] = &R_Y;
				break;
			case Z:
				reg[i] = &R_Z;
				break;
			default:
				reg[i] = NULL;
				break;
		}
	}
}
void makeStringRegisterPointers(int n, unsigned long *hashed, char **reg) {
	for (int i = 0; i < n; i++) {
		switch (hashed[i]) {
			case S:
				reg[i] = R_S;
				break;
			case A:
				reg[i] = R_A;
				break;
			default:
				reg[i] = NULL;
				break;

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
	unsigned long hashedRegisters[SYNTAX_LIMIT];
	int *registers[SYNTAX_LIMIT];
	char *stringRegisters[SYNTAX_LIMIT];
	char buffer2[STRING_SIZE];
	switch (instruction) {
		case OR:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_OR(registers[0], registers[1], registers[2]);
			break;
		case AND:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_AND(registers[0], registers[1], registers[2]);
			break;
		case NOT:
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			F_NOT(registers[0]);
			break;
		case XOR:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_XOR(registers[0], registers[1], registers[2]);
			break;
		case NAND:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_NAND(registers[0], registers[1], registers[2]);
			break;
		case NOR:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_NOR(registers[0], registers[1], registers[2]);
			break;
		case FOPEN:
			openFile(R_IR[0], drive);
			break;
		case MKF:
			createFile(R_IR[0], drive);
			break;
		case RL:
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			readLine(R_S, R_IR[2], 1, drive);
			break;
		case RNC:
			hashRegisters(2, R_IR, hashedRegisters);
			makeRegisterPointers(2, hashedRegisters, registers);
			*registers[1] = readChar(R_IR[2], *registers[0], drive);
			break;
		case PF:
			break;
		case FEX:
			break;
		case FSIZE:
			break;
		case ADD:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_ADD(registers[0], registers[1], registers[2]);
			break;
		case SUB:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_SUB(registers[0], registers[1], registers[2]);
			break;
		case DIV:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_DIV(registers[0], registers[1], registers[2]);
			break;
		case MUL:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_MUL(registers[0], registers[1], registers[2]);
			break;
		case LO:
			hashRegisters(3, R_IR, hashedRegisters);
			makeRegisterPointers(3, hashedRegisters, registers);
			F_LO(registers[0], registers[1], registers[2]);
			break;
		case INC:
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			F_INC(registers[0]);
			break;
		case DEC:
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			F_DEC(registers[0]);
			break;
		case NEG:
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			F_NEG(registers[0]);
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
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			*registers[0] = readFromMemory(-1, 1, R_MAR, -1, R_S);
			break;
		case PMEMLOAD:
			hashRegisters(1, R_IR, hashedRegisters);
			makeRegisterPointers(1, hashedRegisters, registers);
			*registers[0] = readFromMemory(-1, 1, R_SP, -1, R_S);
			break;
		case REGSET:
			hashRegisters(1, R_IR, hashedRegisters);
			if (strcmp(R_IR[1], "INT") == 0) {
				makeRegisterPointers(1, hashedRegisters, registers);
				*registers[0] = atoi(R_IR[2]);
			} else {
				makeStringRegisterPointers(1, hashedRegisters, stringRegisters);
				strcpy(stringRegisters[0], R_IR[2]);
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
			break;
		case OUTPUT:
			hashRegisters(2, R_IR, hashedRegisters);
			if (strcmp(R_IR[0], "INT") == 0) {
				makeRegisterPointers(2, hashedRegisters, registers);
				printf("%d", *registers[1]);
			} else {
				makeStringRegisterPointers(2, hashedRegisters, stringRegisters);
				printf("%s", stringRegisters[1]);
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
			break;
		case CERR:
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

