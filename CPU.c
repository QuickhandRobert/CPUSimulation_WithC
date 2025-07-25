#include <stdio.h>
#include <windows.h>
#include "headers.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
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
HANDLE hMapFile;
CPU_registers *CPU_Registers;
//Global Variables
int memoryAddress[2] = {0}; //Handling multiple apps at once
char errors[ERRORS][STRING_SIZE]; //Error string defined by ERR
procedure proc[MAX_PROCS]; //Procedures (names, startPoints & endPoints)
gotoPoint goto_point[MAX_GOTO_POINTS]; //Gotopoints (PC values)
int prochelper[MAX_PROCS], proc_stack_top = -1; //Procedure handling helper variables
clock_t start; //Used for clock pulse evaluations..
//Error handler
extern enum errors_def error_code;
extern char error_buff[STRING_SIZE];
/**********************************************************
* Func: shared_memory_handler                             *
* Params: const int operationType (REG_INIT, REG_UNINIT)  *
*                                                         *
* Return: none                                            *
* Desc: Initializes or shutdowns the shared memory.       *
**********************************************************/
void shared_memory_handler(const int operationType) {
	switch (operationType) {
		case REG_INIT:
			hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CPU_registers), "Global\\SharedMemory"); //Create a file mapping using windows library tools
			CPU_Registers = (CPU_registers*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CPU_registers)); //Assign the struct CPU_Registers to the mapped file
			CPU_Registers->cp_toggle = false;
			CPU_Registers->cp_differ_toggle = false; //Toggled whenever clockpulse value is changed on the monitor's side
			CPU_Registers->R_C = 1; //Clock accumalator
			CPU_Registers->CPU_Clock = CLOCK_PULSE; //Default value
			break;
		case REG_UNINIT:
			UnmapViewOfFile(CPU_Registers);
			CPU_Registers = NULL;
			CloseHandle(hMapFile);
			hMapFile = NULL;
	}
}
/********************************************
* Func: clock_pulse                         *
* Params: none                              *
*                                           *
* Return: none                              *
* Desc: Emualates a clock cycle             *
********************************************/
void clock_pulse() {
	clock_t current, elapsed = 0, on, off;
	if (CPU_Registers->CPU_Clock < MINIMUM_CLOCK_DURATION) {
		error_code = CPU_LIMIT_EXCEEDED;
		p_error(true);
	}
	//Calculating the duration in which the clock pulse indicator (the knob) should be on or off
	on = CPU_Registers->CPU_Clock * CLOCK_PULSE_TOGGLE_RATIO / 100; //Percentage conversion
	off = CPU_Registers->CPU_Clock - on;
	CPU_Registers->cp_toggle = true; //cp_toggle: Determines whether the knob should be active or not
	while (elapsed < ((CPU_Registers->R_C - 1) * CPU_Registers->CPU_Clock + on)) { //Wait for the duration of clock_t on
		current = clock(); //Get current time
		elapsed = ((unsigned long)(current - start)); //Compare it to when the CPU cycle has started
		if (CPU_Registers->cp_differ_toggle) { //Reset everything if user changed the clock duration value on the monitor's side
			CPU_Registers->R_C = 0;
			start = clock();
			CPU_Registers->cp_differ_toggle = false;
			break;
		}
	}
	CPU_Registers->cp_toggle = false; //Turn of the thing
	//Do the same thing again, this for the duration of clock_t off
	while (elapsed < (CPU_Registers->R_C - 1) * CPU_Registers->CPU_Clock + off + on) {
		current = clock();
		elapsed = ((unsigned long)(current - start));
		if (CPU_Registers->cp_differ_toggle) {
			CPU_Registers->R_C = 0;
			start = clock();
			CPU_Registers->cp_differ_toggle = false;
			break;
		}
	}
	CPU_Registers->cp_differ_toggle = false;
	(CPU_Registers->R_C)++; //Add the clock accumalator
}
/*****************************************************************
* Func: input_delay_handler                                      *
* Params: none                                                   *
*                                                                *
* Return: none                                                   *
* Desc: Accounts for the time spent waiting for user input,      *
*       Required for the clock_pulse logic to work correctly.    *
*****************************************************************/
void input_delay_handler() {
	static bool flag = false;
	static clock_t start_t;
	flag = !flag;
	if (flag)
		start_t = clock(); //Clock the time before user input
	else
		start += clock() - start_t; //Clock the time after user input, and add the difference to clock_t start
}
/********************************************
* Func: fetch                               *
* Params: none                              *
*                                           *
* Return: Hashed value of the read inst     *
* Desc: Emualates CPU fetch                 *
********************************************/
unsigned long fetch() {
	CPU_Registers->step = FETCH; //Set the current status of monitor to FETCH
	for (int i = 0; i < SYNTAX_LIMIT; i++) {
		readFromMemory(1, SYSTEM_RAM, CPU_Registers->R_PC, i, CPU_Registers->R_IR[i]); //Load each parameter of the current instuction into IR
	}
	(CPU_Registers->R_PC)++; //Add the program counter of each FETCH
	clock_pulse(); //Pulse the clock :P
	return readFromMemory(0, 0, CPU_Registers->R_PC - 1, -1, ""); //Return the hashed value
}
/**************************************************************************
* Func: makeRegisterPointers                                              *
* Params: const int n: Number of parameters for the current inst          *
*         registerP *registers: Parameter registers struct                *
*                                                                         *
* Return: none                                                            *
* Desc: Evaluates each one of the register parameters of an instruction   *
**************************************************************************/
void makeRegisterPointers(const int n, registerP *registers) {
	enum registers_hashed registersHashed; //Hashed values of each register's name string
	for (int i = 0; i < n; i++) {
		registers[i].type = M_INT;
		registersHashed = registers[i].hashed;
		switch(registersHashed) {
			case SP:
				registers[i].p = &CPU_Registers->R_SP;
				break;
			case AC:
				registers[i].p = &CPU_Registers->R_AC;
				break;
			case MAR:
				registers[i].p = &CPU_Registers->R_MAR;
				break;
			case MDR:
				registers[i].p = &CPU_Registers->R_MDR;
				break;
			case U:
				registers[i].p = &CPU_Registers->R_U;
				break;
			case V:
				registers[i].p = &CPU_Registers->R_V;
				break;
			case X:
				registers[i].p = &CPU_Registers->R_X;
				break;
			case Y:
				registers[i].p = &CPU_Registers->R_Y;
				break;
			case Z:
				registers[i].p = &CPU_Registers->R_Z;
				break;
			case S:
				registers[i].type = M_STR;
				registers[i].p = CPU_Registers->R_S + registers[i].index;
				break;
			case A:
				registers[i].type = M_STR;
				registers[i].p = CPU_Registers->R_A + registers[i].index;
				break;
			default:
				registers[i].p = NULL;
				break;
		}
	}
}
/*******************************************
* Func: error_def                          *
* Params: none                             *
*                                          *
* Return: none                             *
* Desc: Defines a new error definition     *
*******************************************/
void error_def() {
	char buffer[STRING_SIZE];
	char *res;
	int index;
	strcut(buffer, *CPU_Registers->R_IR, 1, -1); //Remove the E from En (E3 for example), also make a copy, editing IR is prohibited
	if (!isdigit((int)*buffer)) {
		error_code = ERROR_DEFINITION_FAILED_INVALID_INDEX;
		strncpy(error_buff, buffer, STRING_SIZE);
		p_error(false);
		return;
	}
	index = atoi(buffer);
	strcpy(errors[index], CPU_Registers->R_IR[1]);
}
/*******************************************
* Func: const_def                          *
* Params: none                             *
*                                          *
* Return: none                             *
* Desc: Defines a new constant string      *
*******************************************/
void const_def() {
	char buffer[STRING_SIZE];
	char *res;
	int index;
	strcut(buffer, *CPU_Registers->R_IR, 1, -1); //Remove the C from Cn (C3 for example), also make a copy, editing IR is prohibited
	if (!isdigit((int)*buffer)) {
		error_code = CONST_DEFINITION_FAILED_INVALID_INDEX;
		strncpy(error_buff, buffer, STRING_SIZE);
		p_error(false);
		return;
	}
	index = atoi(buffer);
	strncpy(CPU_Registers->CONSTSTR[index], CPU_Registers->R_IR[1], STRING_SIZE);
}
/***********************************************************
* Func: print_error                                        *
* Params: registerP *registers: Array of register structs  *
*                                                          *
* Return: none                                             *
* Desc: Prints an error definition with # as variables     *
***********************************************************/
void print_error(registerP *registers) {
	char buffer[STRING_SIZE];
	char *res;
	int index, len, i, j;
	strcut(buffer, *CPU_Registers->R_IR, 1, -1);
	if (!isdigit((int) *buffer)) { //Check user input
		error_code = ERROR_CALL_FAILED_INVALID_INDEX;
		strncpy(error_buff, buffer, STRING_SIZE);
		p_error(false);
		return;
	}
	index = atoi(buffer);
	len=strlen(errors[index]);
	for (i = 0, j = 1; i < len; i++) {
		if (errors[index][i] == ERR_VAR_INDICATOR) {
			printf("%d", registers[j].p);
			j++;
		} else {
			putchar(errors[index][i]);
		}
	}

}
/*******************************************************
* Func: loadToMemory                                   *
* Params: FILE *drive                                  *
*         char *filename (optional)                    *
*                                                      *
* Return: none                                         *
* Desc: Loads the instructions from disk to RAM        *
*******************************************************/
void loadToMemory(FILE *drive, char *filename) {
	char buffer[LINE_BUFFER_SIZE];
	char *res;
	int i, j;
	unsigned long hashed = 0;
	if (filename && !isStringEmpty(filename))  //Open the given file if filename isn't empty or NULL
		openFile(filename, drive);
	for (i = memoryAddress[1]; j != PROGRAM_ENDED; i++) { //Read the file line by line untill we read "END"
		readLine(buffer, NULL, 0, drive); //Read a line into buffer
		j = decodeInstruction(buffer, i, drive); //Decodes & writes in into SYSTEM_RAM (+checks for comments)
		//if a line is comment, decodeinstruction returns -1, and i decrements, else i increments..
		i = j != PROGRAM_ENDED ? i += j : i;
	}
	memoryAddress[0] = memoryAddress[1]; //Multiple programs handling, TODO: add support for more
	memoryAddress[1] = i - 1;
	proc_init(); // initialize procedures (end, start, names)
	goto_point_init(); //Initialize goto points (start, names)

}
/*******************************************************
* Func: proc_init                                      *
* Params: none                                         *
*                                                      *
* Return: none                                         *
* Desc: Searches SYSTEM_RAM for procedure definitions  *
*       and initializes them for future use            *
*******************************************************/
void proc_init() {
	unsigned long a;
	char buffer[LINE_BUFFER_SIZE];
	enum operations_hashed operationsHashed;
	for (int i = 0, j = 0; i < memoryAddress[1]; i++) {
		if ((operationsHashed = readFromMemory(M_INT, SYSTEM_RAM, i, -1, NULL)) == OP_PROC) { //Search for procedures
			readFromMemory(M_STR, SYSTEM_RAM, i, 0, buffer); //Read procedure's name
			buffer[strlen(buffer) - 1] = '\0'; //Remove ':'
			//Set procedure's parameters
			proc[j].start = i;
			strncpy(proc[j].name, buffer, LINE_BUFFER_SIZE);
			do {
				i++;
			} while ((operationsHashed = readFromMemory(M_INT, SYSTEM_RAM, i, -1, NULL)) != OP_ENDPROC); //Skip over anything that isn't "ENDPROC"
			proc[j].end = i;
			j++;
		}
	}
}
/*******************************************************
* Func: goto_point_init                                *
* Params: none                                         *
*                                                      *
* Return: none                                         *
* Desc: Searches SYSTEM_RAM for goto definitions       *
*       and initializes them for future use            *
*******************************************************/
void goto_point_init() {
	char buffer[LINE_BUFFER_SIZE];
	for (int i = 0, j = 0; i < memoryAddress[1]; i++) {
		if (readFromMemory(M_INT, SYSTEM_RAM, i, -1, NULL) == GOTO_POINT) { //Search for goto point identifier
			readFromMemory(M_STR, SYSTEM_RAM, i, 0, buffer);
			//Set the parameters..
			goto_point[j].pos = i;
			strncpy(goto_point[j].name, buffer, STRING_SIZE);
			j++;
		}
	}
}
/*******************************************************
* Func: findProc                                       *
* Params: const char *name                             *
*                                                      *
* Return: PC value pointing to the start of the Proc   *
*******************************************************/
int findProc(const char *name) {
	int len = strlen(name);
	char buffer[STRING_SIZE];
	strncpy(buffer, name, STRING_SIZE);
	buffer[len - 1] = buffer[len - 1] == ':' ? '\0' : buffer[len - 1]; //Remove ':'
	for (int i = 0; i < MAX_PROCS; i++)
		if (strcmp(proc[i].name, buffer) == 0)
			return i;
	return NOT_FOUND;
}
/*******************************************************
* Func: findGotoPoint                                  *
* Params: const char *name                             *
*                                                      *
* Return: PC value pointing to given goto point        *
*******************************************************/
int findGotoPoint(const char *name) {
	for (int i = 0; i < MAX_GOTO_POINTS; i++)
		if (strcmp(goto_point[i].name, name) == 0)
			return goto_point[i].pos;
	return NOT_FOUND;
}
/**************************************************************************************
* Func: decodeInstruction                                                             *
* Params: char *buffer                                                                *
*         const int index                                                             *
*         FILE *drive                                                                 *
*                                                                                     *
* Return: LINE_COMMENT, LINE_OK or PROGRAM_ENDED                                      *
* Desc: Tokenizes a read buffer into sectoins, and writes them to the SYSTEM_RAM      *
**************************************************************************************/
int decodeInstruction(char *buffer, const int index, FILE *drive) {
	unsigned long hashed = 0;
	char *res, *c;
	int j;
	removeNewLine(buffer);
	if (c = strchr(buffer, '\t'))
		*c = '\0'; //Handling tabs at the end of each command
	if (*buffer == GOTO_POINT_INDICATOR) { //Check for goto lines
		buffer++;
		writeToMemory(M_STR, SYSTEM_RAM, index, -1, GOTO_POINT, buffer);
		writeToMemory(M_STR, SYSTEM_RAM, index, 0, S_NULL, buffer);
		return LINE_OK;
	}
	res = strtok_fixed(buffer, " ", '\"');
	hashed = hashStr(res); //Hash the current instruction
	if (*res == COMMENT_SIGN) { //Check for commented line
		return LINE_COMMENT;
	}
	for (j = 0; hashed != END && hashed != ENDN && res != NULL; j++) { //Keep tokenizing the buffer till it ends
		if (!j) {
			if (*res == COMMENT_SIGN)
				return LINE_COMMENT; //End reading, rest is commented
			writeToMemory(0, SYSTEM_RAM, index, -1, hashed, ""); //If not, write the hashed value of the current instruction into index -1 of SYSTEM_RAM
		} else {
			if (*res == COMMENT_SIGN) //If a COMMENT_SIGN is encountered, and we're not on the fist parameter (the instruction itself)
				return LINE_OK;
			writeToMemory(1, SYSTEM_RAM, index, j - 1, 0, res); //Write the instruction into SYSTEM_RAM, [index][j]
		}
		res = strtok_fixed(NULL, " ", '\"');
	}
	if (hashed == END || hashed == ENDN) return PROGRAM_ENDED;
	return LINE_OK;
}
/*********************************************
* Func: shift_IR                             *
* Params: bool i (FORWARD or BACK)           *
*                                            *
* Return: none                               *
* Desc: Used for CRUN & RUN instructions     *
*       Shifts the IR Back or forward        *
*********************************************/
void shift_IR(bool i) {
	switch(i) {
		case BACK:
			for (int j = 1; j < SYNTAX_LIMIT; j++)
				strncpy(CPU_Registers->R_IR[j - 1], (CPU_Registers->R_IR)[j], STRING_SIZE);
			break;
		case FORWARD:
			for (int j = 0; j < SYNTAX_LIMIT - 1; j++)
				strncpy(CPU_Registers->R_IR[j + 1], CPU_Registers->R_IR[j], STRING_SIZE);
	}
}
/*****************************************************************
* Func: decode_execute                                           *
* Params: unsigned long instruction: Hashed value                *
*         FILE *drive                                            *
*                                                                *
* Return: none                                                   *
* Desc: Main function of the systems, handles DECODE & EXECUTE   *
*****************************************************************/
void decode_execute(unsigned long instruction, FILE *drive) {
	CPU_Registers->step = DECODE; // Set the current status of the monitor to DECODE
	enum operations_hashed operationsHashed;
	registerP registers[SYNTAX_LIMIT];
	unsigned long inst;
	char buffer2[STRING_SIZE];
	char buffer0[1][STRING_SIZE];
	operationsHashed = instruction;
	int copied_value; //Used for the COPY inst
	switch (operationsHashed) {
		//----------------------------------
		//Logic operations
		case OP_OR:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "OR");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_OR(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_AND:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "AND");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_AND(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_NOT:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "NOT");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_NOT(registers[0].p);
			break;
		case OP_XOR:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "XOR");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_XOR(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_NAND:
			//Set monitor's parameters
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_NAND(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_NOR:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "NOR");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_NOR(registers[0].p, registers[1].p, registers[2].p);
			break;
		//----------------------------------
		//File operations
		case OP_FOPEN:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "FOPEN");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			openFile(CPU_Registers->R_IR[0], drive);
			break;
		case OP_MKF:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "MKF");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			createFile(CPU_Registers->R_IR[0], drive);
			break;
		case OP_RL:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "RL"); //RL: read line
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			readLine((char *)registers[0].p, CPU_Registers->R_IR[2], 1, drive);
			break;
		case OP_RNC:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "RNC"); //RNC: read n'th character
			CPU_Registers->R_IR_INST.inst_params = 2;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			*(char *)registers[1].p = readChar(CPU_Registers->R_IR[2], *(registers[0].p), drive);
			break;
		case OP_FEX:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "FEX"); //FEX: file exists?
			CPU_Registers->R_IR_INST.inst_params = 2;
			clock_pulse();
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			*registers[0].p = fileSearch(CPU_Registers->R_IR[1]) == FILE_FAILED ? false : true;
			CPU_Registers->step = EXECUTE;
			break;
		case OP_FSIZE: //TODO: impelement it
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "FSIZE");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			break;
		//----------------------------------
		//Mathematical operations
		case OP_ADD:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "ADD");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_ADD(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_SUB:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "SUB");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_SUB(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_DIV:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "DIV");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_DIV(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_MUL:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "MUL");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_MUL(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_LO:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "LO"); //LO: leftover (%)
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_LO(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_INC:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "INC"); //INC: increment
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_INC(registers[0].p);
			break;
		case OP_DEC:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "DEC"); //DEC: decrement
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_DEC(registers[0].p);
			break;
		case OP_NEG:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "NEG"); //NEG: negate
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_NEG(registers[0].p);
			break;
		//----------------------------------
		//Memory Operations
		case OP_COPY: //Copy memory values (from given value of a provided register to whatever address is in MAR)
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "COPY");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			copied_value = readFromMemory(M_INT, USER_RAM, *registers[0].p, S_NULL, NULL);
			writeToMemory(M_INT, USER_RAM, CPU_Registers->R_MAR, S_NULL, copied_value, NULL);
			break;
		case OP_FLUSH:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "FLUSH"); //Flush a given memory address
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			strcut(buffer2, CPU_Registers->R_IR[0], 1, -1);
			flushMemoryAddress(atoi(buffer2), 1);
			break;
		case OP_CLEAR:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "CLEAR"); //Clear everything
			CPU_Registers->R_IR_INST.inst_params = 0;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			memoryInit(1);
			break;
		case OP_MEMWRITE: //Write whatever's in MDR into address MAR
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "MEMWRITE");
			CPU_Registers->R_IR_INST.inst_params = 0;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			writeToMemory(S_NULL, USER_RAM, CPU_Registers->R_MAR, S_NULL, CPU_Registers->R_MDR, NULL);
			CPU_Registers->R_SP = CPU_Registers->R_MAR; // Set stack pointer type shit
			break;
		case OP_PMEMWRITE: //Write whatever's in MDR into address AC
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "PMEMWRITE");
			CPU_Registers->R_IR_INST.inst_params = 0;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			writeToMemory(S_NULL, USER_RAM, CPU_Registers->R_SP, S_NULL, CPU_Registers->R_MDR, NULL);
			break;
		case OP_MEMLOAD:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "MEMLOAD");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			CPU_Registers->R_MDR = (int)readFromMemory(S_NULL, USER_RAM, CPU_Registers->R_MAR, S_NULL, NULL);
			break;
		case OP_PMEMLOAD:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "PMEMLOAD");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			CPU_Registers->R_MDR = (int)readFromMemory(S_NULL, USER_RAM, CPU_Registers->R_SP, S_NULL, NULL);
			break;
		//----------------------------------
		//Register Operations
		case OP_REGSET: //Set a given register's value to a provided integer (or character)
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "REGSET");
			CPU_Registers->R_IR_INST.inst_params = 2;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (isdigit(*CPU_Registers->R_IR[1]) || *CPU_Registers->R_IR[1] == '-') //Is the value an integer?
				*registers[0].p = atoll(CPU_Registers->R_IR[1]);
			else //Or an ASCII character?
				*registers[0].p = (int)*CPU_Registers->R_IR[1];
			break;
		case OP_REGCOPY:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "REGCOPY");
			CPU_Registers->R_IR_INST.inst_params = 2;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			*registers[1].p = *registers[0].p;
			break;
		case OP_EQ: //Are two register's values equal?
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "EQ");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_EQ(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_SHIFTFORWARD:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "SHIFTFORWARD");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			*registers[2].p = *registers[0].p >> *registers[1].p;
			break;
		case OP_SHIFTBACK:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "SHIFTBACK");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			*registers[2].p = *registers[0].p << *registers[1].p;
			break;
		//----------------------------------
		//IO
		case OP_INPUT: //Read a value from stdin (single character)
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "INPUT");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			input_delay_handler();
			*registers[0].p = getchar_fixed();
			input_delay_handler();
			break;
		case OP_OUTPUT: //Write a value to stdout (single character ASCII)
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "OUTPUT");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			if (**CPU_Registers->R_IR == 'C') // Cn (Constant Strings)
				registers[0].type = M_CONST;
			else {
				hashRegisters(1, CPU_Registers->R_IR, registers);
				makeRegisterPointers(1, registers);
			}
			switch(registers[0].type) {
				case M_CONST: //TODO: decide wether we remove this or not
					strcut(buffer2, *CPU_Registers->R_IR, 1, -1);
					if (isdigit((int)*buffer2))
						printf("%s", CPU_Registers->CONSTSTR[atoi(buffer2)]);
					else {
						strcut(buffer2, buffer2, 1, -1);
						strcpy(buffer0[0], buffer2);
						hashRegisters(1, buffer0, registers);
						makeRegisterPointers(1, registers);
						printf("%s", CPU_Registers->CONSTSTR[*registers[0].p]);
					}
					break;
				default:
					putchar(*registers[0].p);
			}
			break;
		case OP_CLS: //Clear console
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "CLS");
			CPU_Registers->R_IR_INST.inst_params = 0;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			system("cls");
			break;
		//----------------------------------
		//Program logic
		case OP_GOTO:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "GOTO");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			CPU_Registers->R_PC = findGotoPoint(CPU_Registers->R_IR[0]) + 1;
			break;
		case OP_RUN: //TODO: Executable files support
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "RUN");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			inst = hashStr(CPU_Registers->R_IR[0]);
			shift_IR(BACK);
			decode_execute(inst, drive);
			break;
		case OP_CRUN: //TODO: Executable files support
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "CRUN");
			CPU_Registers->R_IR_INST.inst_params = 2;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (*registers[0].p) {
				inst = hashStr(CPU_Registers->R_IR[1]);
				shift_IR(BACK);
				shift_IR(BACK);
				decode_execute(inst, drive);
			}
			break;
		case OP_SHUTDOWN:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "SHUTDOWN");
			CPU_Registers->R_IR_INST.inst_params = 0;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			system_shutdown(drive);
			break;
		case OP_HIBERNATE: //TODO: implement this
			break;
		//----------------------------------
		//Definitions
		case OP_CONST:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "CONST");
			CPU_Registers->R_IR_INST.inst_params = 2;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			const_def();
			break;
		case OP_ERR:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "ERR");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			error_def();
			break;
		case OP_CERR:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "CERR");
			CPU_Registers->R_IR_INST.inst_params = 3;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			hashRegisters(7, CPU_Registers->R_IR, registers);
			makeRegisterPointers(7, registers);
			print_error(registers);
			break;
		case OP_PROC:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "PROC");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			CPU_Registers->R_PC = proc[findProc(CPU_Registers->R_IR[0])].end;
			(CPU_Registers->R_PC)++;
			break;
		case OP_RUNPROC:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "RUNPROC");
			CPU_Registers->R_IR_INST.inst_params = 1;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			proc_stack_top++;
			prochelper[proc_stack_top] = CPU_Registers->R_PC;
			strcpy(buffer2, CPU_Registers->R_IR[0]);
			CPU_Registers->R_PC = proc[findProc(buffer2)].start;
			(CPU_Registers->R_PC)++;
			break;
		case OP_ENDPROC:
			//Set monitor's parameters
			strcpy(CPU_Registers->R_IR_INST.inst_name, "ENDRROC");
			CPU_Registers->R_IR_INST.inst_params = 0;
			clock_pulse();
			CPU_Registers->step = EXECUTE;
			CPU_Registers->R_PC = prochelper[proc_stack_top];
			proc_stack_top--;
			break;
		case OP_DEBUG_BPOINT:
			strcpy(CPU_Registers->R_IR_INST.inst_name, "DEBUG");
			CPU_Registers->R_IR_INST.inst_params = 0;
			printf("\nDebugging breakpoint reached!\n");
			break;
	}
	clock_pulse();
}
void runCPU(FILE *drive) {
	start = clock(); //Set start time
	unsigned long inst;
	CPU_Registers->R_PC = memoryAddress[0]; //Set the PC value to the start of the currently running program...
	
	//CPU Cycle
	while(CPU_Registers->R_PC < memoryAddress[1]) { //FETCH till the program ends...
		inst = fetch();
		decode_execute(inst, drive);
	}
}


