#include <stdio.h>
#include <windows.h>
#include "headers.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <process.h>
#include <conio.h>
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
HANDLE hStdin;
DWORD echo_off, echo_on;
CPU_registers_t *CPU_Registers;
program_t *programs;
//Global Variables
int programsTop = 0;
long pc_max = 0;
char errors[ERRORS][STRING_SIZE]; //Error string defined by ERR
procedure_t proc[MAX_PROGRAMS][MAX_PROCS]; //Procedures (names, startPoints & endPoints)
gotoPoint_t goto_point[MAX_PROGRAMS][MAX_GOTO_POINTS]; //Gotopoints (PC values)
int prochelper[MAX_PROGRAMS][MAX_PROCS], proc_stack_top[MAX_PROGRAMS] = {0}; //Procedure handling helper variables
clock_t start; //Used for clock pulse evaluations..
//Error handler
extern enum errors_def error_code;
extern char error_buff[STRING_SIZE];
//System Log
FILE *debugOutput;
/**********************************************************
* Func: shared_memory_handler                             *
* Params: const int operationType (REG_INIT, REG_UNINIT)  *
*                                                         *
* Return: none                                            *
* Desc: Initializes or shutdowns the shared memory.       *
**********************************************************/
void shared_memory_handler(const int operationType, const bool restart_flag) {
	switch (operationType) {
		case REG_INIT:
			hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CPU_registers_t), "Global\\SharedMemory"); //Create a file mapping using windows library tools
			CPU_Registers = (CPU_registers_t*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CPU_registers_t)); //Assign the struct CPU_Registers to the mapped file
			CPU_Registers->cp_toggle = false;
			CPU_Registers->cp_differ_toggle = false; //Toggled whenever clockpulse value is changed on the monitor's side
			CPU_Registers->R_C = 1; //Clock accumalator
			CPU_Registers->CPU_Clock = CLOCK_PULSE; //Default value
			CPU_Registers->power_state = restart_flag ? true : false;
			CPU_Registers->p_id = GetCurrentProcessId();
			CPU_Registers->p_id_trigger = true;
			//Window mode initializations
			hStdin = GetStdHandle(STD_INPUT_HANDLE);
			GetConsoleMode(hStdin, &echo_on);
			echo_on |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; //Enabled for moving cursor freely n stuff
			SetConsoleMode(hStdin, echo_on);
			echo_off = echo_on;
			echo_off &= ~(ENABLE_ECHO_INPUT); //Disable echo input\
			//Programs struct
			programs = (program_t *)malloc(sizeof(program_t) * MAX_PROGRAMS);
			programs->start = 0;
			break;
		case REG_UNINIT:
			UnmapViewOfFile(CPU_Registers);
			CPU_Registers = NULL;
			CloseHandle(hMapFile);
			hMapFile = NULL;
			free(programs);
	}
}
/********************************************
* Func: wait_for_power                      *
* Params: bool flag                         *
*                                           *
* Return: None                              *
* Desc: won't wait if flag                  *
********************************************/
void wait_for_power(bool flag) {
	while (!flag && !CPU_Registers->power_state)
		Sleep(WATCH_FOR_POWEROFF_WAIT_INTERVAL); //Prevent flooding
}
/******************************************************
* Func: watch_for_poweroff                            *
* Parmams: none                                       *
*                                                     *
* Return: none                                        *
* Desc: Checks if a shutdown or restart is initiated  *
*       On the monitor's side                         *
******************************************************/
void watch_for_poweroff() {
	while (true) {
		if (!CPU_Registers->power_state) {
			system_shutdown();
			return;
		}
		if (CPU_Registers->hibernate_trigger) {
			CPU_Registers->hibernate_trigger = false;
			CPU_Registers->power_state = false;
			system_hibernate();
			return;
		}
		if (CPU_Registers->restart_trigger && CPU_Registers->power_state) { //System is on and a restart is requested, then...
			CPU_Registers->restart_trigger = false;
			system_restart();
			return;
		}
		Sleep(WATCH_FOR_POWEROFF_WAIT_INTERVAL);
	}
	return;
}
/********************************************
* Func: isSystemOn                          *
* Params: none                              *
*                                           *
* Return: bool                              *
********************************************/
bool isSystemOn() {
	return CPU_Registers->power_state;
}
/********************************************
* Func: clock_pulse                         *
* Params: none                              *
*                                           *
* Return: none                              *
* Desc: Emualates a clock cycle             *
********************************************/
static inline void clock_pulse() {
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
static void input_delay_handler() {
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
static unsigned long fetch() {
	CPU_Registers->step = FETCH; //Set the current status of monitor to FETCH
	for (int i = 0; i < SYNTAX_LIMIT; i++) {
		readFromMemory(M_STR, SYSTEM_RAM, CPU_Registers->R_PC, i, CPU_Registers->R_IR[i]); //Load each parameter of the current instuction into IR
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
void makeRegisterPointers(const int n, registerP_t *registers) {
	enum registers_hashed registersHashed; //Hashed values of each register's name string
	for (int i = 0; i < n; i++) {
		registers[i].type = M_INT;
		registersHashed = registers[i].hashed;
		switch(registersHashed) {
			case PC:
				registers[i].p = &CPU_Registers->R_PC;
				break;
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
static void error_def() {
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
static void const_def() {
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
static void print_error(registerP_t *registers) {
	char buffer[STRING_SIZE], buffer0[STRING_SIZE];
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
		if (errors[index][i] == ERR_INT_INDICATOR) {
			printf("%lld", *registers[j].p);
			j++;
		} else if (errors[index][i] == ERR_STR_INDICATOR) {
			intPtoString(buffer0, registers[j].p, INTP_TO_CHARP,STRING_SIZE);
			printf("%s", buffer0);
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
	int i = 0, j = 0;
	unsigned long hashed = 0;
	if (filename && !isStringEmpty(filename))  //Open the given file if filename isn't empty or NULL
		openFile(filename, drive);
	for (i = (programs + programsTop)->start; j != PROGRAM_ENDED; i++) { //Read the file line by line untill we read "END"
		readLine(buffer, NULL, 0, drive); //Read a line into buffer
		j = decodeInstruction(buffer, i, drive); //Decodes & writes in into SYSTEM_RAM (+checks for comments)
		//if a line is comment, decodeinstruction returns -1, and i decrements, else i increments..
		i = j != PROGRAM_ENDED ? i + j : i;
	}

	(programs + programsTop + 1)->start = i;
	proc_init(); // initialize procedures (end, start, names)
	goto_point_init(); //Initialize goto points (start, names)
	programsTop++;
	pc_max += i;
}
static void unloadFromMemory() {
	pc_max = (programs + programsTop) -> start;
	programsTop--;
}
/*******************************************************
* Func: proc_init                                      *
* Params: none                                         *
*                                                      *
* Return: none                                         *
* Desc: Searches SYSTEM_RAM for procedure definitions  *
*       and initializes them for future use            *
*******************************************************/
static void proc_init() {
	unsigned long a;
	char buffer[LINE_BUFFER_SIZE];
	enum operations_hashed operationsHashed;
	int i, j;
	for (i = (programs + programsTop)->start, j = 0; i < (programs + programsTop + 1)->start; i++) {
		if ((operationsHashed = readFromMemory(M_INT, SYSTEM_RAM, i, -1, NULL)) == OP_PROC) { //Search for procedures
			readFromMemory(M_STR, SYSTEM_RAM, i, 0, buffer); //Read procedure's name
			buffer[strlen(buffer) - 1] = '\0'; //Remove ':'
			//Set procedure's parameters
			proc[programsTop][j].start = i;
			strncpy(proc[programsTop][j].name, buffer, LINE_BUFFER_SIZE);
			do {
				i++;
			} while ((operationsHashed = readFromMemory(M_INT, SYSTEM_RAM, i, -1, NULL)) != OP_ENDPROC); //Skip over anything that isn't "ENDPROC"
			proc[programsTop][j].end = i;
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
static void goto_point_init() {
	int i, j;
	char buffer[LINE_BUFFER_SIZE];
	for (i = (programs + programsTop)->start, j = 0; i < (programs + programsTop + 1)->start; i++) {
		if (readFromMemory(M_INT, SYSTEM_RAM, i, -1, NULL) == GOTO_POINT) { //Search for goto point identifier
			readFromMemory(M_STR, SYSTEM_RAM, i, 0, buffer);
			//Set the parameters..
			goto_point[programsTop][j].pos = i;
			strncpy(goto_point[programsTop][j].name, buffer, STRING_SIZE);
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
static int findProc(const char *name) {
	int len = strlen(name);
	char buffer[STRING_SIZE];
	strncpy(buffer, name, STRING_SIZE);
	buffer[len - 1] = buffer[len - 1] == ':' ? '\0' : buffer[len - 1]; //Remove ':'
	for (int i = 0; i < MAX_PROCS; i++)
		if (strcmp(proc[programsTop - 1][i].name, buffer) == 0)
			return i;
	return NOT_FOUND;
}
/*******************************************************
* Func: findGotoPoint                                  *
* Params: const char *name                             *
*                                                      *
* Return: PC value pointing to given goto point        *
*******************************************************/
static int findGotoPoint(const char *name) {
	for (int i = 0; i < MAX_GOTO_POINTS; i++)
		if (strcmp(goto_point[programsTop - 1][i].name, name) == 0)
			return goto_point[programsTop - 1][i].pos;
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
static int decodeInstruction(char *buffer, const int index, FILE *drive) {
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
	if (isStringEmpty(buffer)) {
		return LINE_COMMENT;
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
static void shift_IR(bool dir) {
	switch(dir) {
		case BACK:
			for (int j = 1; j < SYNTAX_LIMIT; j++)
				strncpy(CPU_Registers->R_IR[j - 1], (CPU_Registers->R_IR)[j], STRING_SIZE);
			break;
		case FORWARD:
			for (int j = 0; j < SYNTAX_LIMIT - 1; j++)
				strncpy(CPU_Registers->R_IR[j + 1], CPU_Registers->R_IR[j], STRING_SIZE);
	}
}
/************************************************************
* Func: inst_init                                           *
* Params: const char *inst_name                             *
*         const int params_cnt                              *
*                                                           *
* Return: none                                              *
* Desc: Initlializations done before each inst's execution  *
************************************************************/
static inline void inst_init(const char *inst_name, const int params_cnt) {
	strcpy(CPU_Registers->R_IR_INST.inst_name, inst_name);
	CPU_Registers->R_IR_INST.inst_params = params_cnt;
	clock_pulse();
	CPU_Registers->step = EXECUTE;
}
/************************************************************
* Func: pgfile_write_registers                              *
* Params: FILE *fp                                          *
*                                                           *
* Return: none                                              *
* Desc: Writes necessary values from CPU_Registers into fp  *
************************************************************/
void pgfile_write_registers(FILE *fp) {
	unsigned long inst_temp;
	if ((inst_temp = readFromMemory(M_INT, SYSTEM_RAM, CPU_Registers->R_PC - 1, -1, NULL)) == OP_INPUT || inst_temp == OP_GETKEY)
		(CPU_Registers->R_PC)--;
	fwrite(&CPU_Registers->R_PC, sizeof(long long int), PAGED_REGISTERS_COUNT, fp); //PC to Z
	fwrite(CPU_Registers->R_S, sizeof(long long int), REG_STRING_SIZE * 2, fp); //S and A
	fwrite(CPU_Registers->CONSTSTR, sizeof(CPU_Registers->CONSTSTR), 1, fp);
	fwrite(programs, sizeof(program_t), MAX_PROGRAMS, fp);
	fwrite(errors, sizeof(errors), 1, fp);
	fwrite(proc, sizeof(proc), 1, fp);
	fwrite(goto_point, sizeof(goto_point), 1, fp);
	fwrite(prochelper, sizeof(prochelper), 1, fp);
	fwrite(proc_stack_top, sizeof(proc_stack_top), 1, fp);
	fwrite(&programsTop, sizeof(programsTop), 1, fp);
	fwrite(&pc_max, sizeof(pc_max), 1, fp);
	return;
}
/************************************************************
* Func: pgfile_write_registers                              *
* Params: FILE *fp                                          *
*                                                           *
* Return: none                                              *
* Desc: Loads values from fp into CPU_Registers             *
************************************************************/
void pgfile_load_registers(FILE *fp) {
	fread(&CPU_Registers->R_PC, sizeof(long long int), PAGED_REGISTERS_COUNT, fp); //PC to Z
	fread(CPU_Registers->R_S, sizeof(long long int), REG_STRING_SIZE * 2, fp); //S and A
	fread(CPU_Registers->CONSTSTR, sizeof(CPU_Registers->CONSTSTR), 1, fp);
	fread(programs, sizeof(program_t), MAX_PROGRAMS, fp);
	fread(errors, sizeof(errors), 1, fp);
	fread(proc, sizeof(proc), 1, fp);
	fread(goto_point, sizeof(goto_point), 1, fp);
	fread(prochelper, sizeof(prochelper), 1, fp);
	fread(proc_stack_top, sizeof(proc_stack_top), 1, fp);
	fread(&programsTop, sizeof(programsTop), 1, fp);
	fread(&pc_max, sizeof(pc_max), 1, fp);
}
/************************************************************
* Func: pgfile_write_console_buffer                         *
* Params: FILE *fp                                          *
*                                                           *
* Return: COORD cursor_position                             *
* Desc: Writes console buffer into fp                       *
************************************************************/
COORD pgfile_write_console_buffer(FILE *fp) { //TODO: Implement unix support, Uses Windows API currently
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	CHAR_INFO *buff = (CHAR_INFO *)malloc(csbi.dwSize.X * csbi.dwSize.Y * sizeof(CHAR_INFO));
	COORD buff_size = csbi.dwSize;
	COORD buff_coords = {0, 0};
	SMALL_RECT read_region = {0, 0, csbi.dwSize.X - 1, csbi.dwSize.Y - 1};
	ReadConsoleOutput(hConsole, buff, buff_size, buff_coords, &read_region);
	fwrite(&buff_size, sizeof(COORD), 1, fp);
	fwrite(&buff_coords, sizeof(COORD), 1, fp);
	fwrite(&read_region, sizeof(SMALL_RECT), 1, fp);
	fwrite(buff, sizeof(CHAR_INFO), csbi.dwSize.X * csbi.dwSize.Y, fp);
	free(buff);
	return csbi.dwCursorPosition;
}
/************************************************************
* Func: pgfile_load_console_buffer                          *
* Params: FILE *fp                                          *
*                                                           *
* Return: none                                              *
* Desc: Loads console buffer & cursor position from fp      *
************************************************************/
void pgfile_load_console_buffer(FILE *fp){
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD buff_size;
	fread(&buff_size, sizeof(COORD), 1, fp);
	COORD buff_coords;
	fread(&buff_coords, sizeof(COORD), 1, fp);
	SMALL_RECT read_region;
	fread(&read_region, sizeof(SMALL_RECT), 1, fp);
	CHAR_INFO *buff = (CHAR_INFO *)malloc(buff_size.X * buff_size.Y * sizeof(CHAR_INFO));
	fread(buff, sizeof(CHAR_INFO), buff_size.X * buff_size.Y, fp);
	COORD cursor_pos;
	fread(&cursor_pos, sizeof(COORD), 1, fp);
	WriteConsoleOutput(hConsole, buff, buff_size, buff_coords, &read_region);
	SetConsoleCursorPosition(hConsole, cursor_pos);
	free(buff);
}
/*************************
* Func: pgfile_write     *
* Params: none           *
*                        *
* Return: none           *
*************************/
void pgfile_write() {
	DWORD f_attributes = 0;
	FILE *fp = fopen(PAGEFILE_FILENAME,  "wb");
	if (SET_FILE_HIDDEN_ATTRIBUTE){
		f_attributes = GetFileAttributes(PAGEFILE_FILENAME);
		f_attributes |= (FILE_ATTRIBUTE_HIDDEN);
		SetFileAttributes(PAGEFILE_FILENAME, f_attributes);
	}
	size_t reg_start = (PAGEFILE_HEADER_PARAMS_COUNT * sizeof(size_t)), sys_mem_start, usr_mem_start, console_start, cursor_pos_start;
	COORD cursor_pos;
	fseek(fp, reg_start, SEEK_SET);
	pgfile_write_registers(fp);
	sys_mem_start = ftell(fp);
	pgfile_memory_write(fp, SYSTEM_RAM);
	usr_mem_start = ftell(fp);
	pgfile_memory_write(fp, USER_RAM);
	console_start = ftell(fp);
	cursor_pos = pgfile_write_console_buffer(fp);
	cursor_pos_start = ftell(fp);
	fwrite(&cursor_pos, sizeof(COORD), 1, fp);
	pgfile_write_drive_pos(fp);
	rewind(fp);
	fwrite(&reg_start, sizeof(size_t), 1, fp);
	fwrite(&sys_mem_start, sizeof(size_t), 1, fp);
	fwrite(&usr_mem_start, sizeof(size_t), 1, fp);
	fwrite(&console_start, sizeof(size_t), 1, fp);
	fwrite(&cursor_pos_start, sizeof(size_t), 1, fp);
	fclose(fp);
	return;
}
/*************************
* Func: pgfile_load      *
* Params: none           *
*                        *
* Return: none           *
*************************/
void pgfile_load() {
	FILE *fp = fopen(PAGEFILE_FILENAME, "rb");
	size_t reg_start, sys_mem_start, usr_mem_start, console_start, cursor_pos_start;
	pgfile_header_init(&reg_start, &sys_mem_start, &usr_mem_start, &console_start, &cursor_pos_start, fp);
	fseek(fp, SEEK_SET, reg_start);
	pgfile_load_registers(fp);
	fseek(fp, SEEK_SET, sys_mem_start);
	pgfile_memory_load(fp, usr_mem_start, SYSTEM_RAM);
	pgfile_memory_load(fp, console_start, USER_RAM);
	pgfile_load_console_buffer(fp);
	pgfile_load_drive_pos(fp);
	fclose(fp);
}
/************************************************************
* Func: pgfile_header_init                                  *
* Params: size_t *reg_start                                 *
*         size_t *sys_mem_start                             *
*         size_t *usr_mem_start                             *  
*         size_t *console_start                             *
*         size_t *cursor_pos_start                          *
*         FILE *fp                                          *
*                                                           *
* Return: none                                              *
* Desc: Loads header values from fp                         *
************************************************************/
void pgfile_header_init(size_t *reg_start, size_t *sys_mem_start, size_t *usr_mem_start, size_t *console_start, size_t *cursor_pos_start, FILE *fp) {
	fread(reg_start, sizeof(size_t), 1, fp);
	fread(sys_mem_start, sizeof(size_t), 1, fp);
	fread(usr_mem_start, sizeof(size_t), 1, fp);
	fread(console_start, sizeof(size_t), 1, fp);
	fread(cursor_pos_start, sizeof(size_t), 1, fp);
	return;
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
	registerP_t registers[SYNTAX_LIMIT];
	unsigned long inst;
	char *rename_buff0, *rename_buff1;
	char buffer2[STRING_SIZE];
	char buffer0[1][STRING_SIZE];
	operationsHashed = instruction;
	int copied_value; // Used for the COPY inst
	int ch; // Used for GETKEY inst
	switch (operationsHashed) {
		//----------------------------------
		// Logic operations
		case OP_OR:
			inst_init("OR", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_OR(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_AND:
			inst_init("AND", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_AND(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_NOT:
			inst_init("NOT", 3);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_NOT(registers[0].p);
			break;
		case OP_XOR:
			inst_init("XOR", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_XOR(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_NAND:
			inst_init("NAND", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_NAND(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_NOR:
			inst_init("NOR", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_NOR(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_BITAND:
			inst_init("BITAND", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_BITAND(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_BITOR:
			inst_init("BITOR", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_BITOR(registers[0].p, registers[1].p, registers[2].p);
			break;
		//----------------------------------
		// File operations
		case OP_FOPEN:
			inst_init("FOPEN", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (registers[0].p) {
				intPtoString(buffer2, registers[0].p, INTP_TO_CHARP, STRING_SIZE);
				openFile(buffer2, drive);
			} else
				openFile(CPU_Registers->R_IR[0], drive);
			break;
		case OP_MKF:
			inst_init("MKF", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (registers[0].p) {
				intPtoString(buffer2, registers[0].p, INTP_TO_CHARP, STRING_SIZE);
				createFile(buffer2, drive);
			} else
				createFile(CPU_Registers->R_IR[0], drive);
			break;
		case OP_RM:
			inst_init("RM", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (registers[0].p) {
				intPtoString(buffer2, registers[0].p, INTP_TO_CHARP, STRING_SIZE);
				deleteFile(buffer2, drive);
			} else
				deleteFile(CPU_Registers->R_IR[0], drive);
			break;
		case OP_RENAME:
			inst_init("RENAME", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			if (registers[0].p) {
				intPtoString(buffer0[0], registers[0].p, INTP_TO_CHARP, STRING_SIZE);
				rename_buff0 = buffer0[0];
			} else
				rename_buff0 = CPU_Registers->R_IR[0];
			if (registers[1].p) {
				intPtoString(buffer2, registers[1].p, INTP_TO_CHARP, STRING_SIZE);
				rename_buff1 = buffer2;
			} else
				rename_buff1 = CPU_Registers->R_IR[1];
			renameFile(rename_buff0, rename_buff1, drive);
			break;
		case OP_RNC:
			inst_init("RNC", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			*registers[1].p = readChar(NULL, *(registers[0].p), drive);
			break;
		case OP_WNC:
			inst_init("WNC", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			writeChar(NULL, (char)*registers[1].p, *registers[0].p, drive);
			break;
		case OP_FEX:
			inst_init("FEX", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			if (registers[1].p) {
				intPtoString(buffer2, registers[1].p, INTP_TO_CHARP, STRING_SIZE);
				*registers[0].p = !(fileSearch(buffer2) == FILE_FAILED);
			} else
				*registers[0].p = !(fileSearch(CPU_Registers->R_IR[1]) == FILE_FAILED);
			break;
		case OP_FSIZE:
			inst_init("FSIZE", 1);
			break;
		case OP_GETFILEINFO:
			inst_init("GETFILEINFO", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			getFileInfo(registers[1].p, *registers[0].p, FILENAME_SIZE);
			break;
		//----------------------------------
		// Mathematical operations
		case OP_ADD:
			inst_init("ADD", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_ADD(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_SUB:
			inst_init("SUB", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_SUB(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_DIV:
			inst_init("DIV", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_DIV(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_MUL:
			inst_init("MUL", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_MUL(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_LO:
			inst_init("LO", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_LO(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_INC:
			inst_init("INC", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_INC(registers[0].p);
			break;
		case OP_DEC:
			inst_init("DEC", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_DEC(registers[0].p);
			break;
		case OP_NEG:
			inst_init("NEG", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			F_NEG(registers[0].p);
			break;
		//----------------------------------
		// Memory Operations
		case OP_COPY:
			inst_init("COPY", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			writeToMemory(M_INT, USER_RAM, CPU_Registers->R_MAR, S_NULL, readFromMemory(M_INT, USER_RAM, *registers[0].p, S_NULL, NULL), NULL);
			break;
		case OP_FLUSH:
			inst_init("FLUSH", 0);
			flushMemoryAddress(CPU_Registers->R_MAR, USER_RAM);
			break;
		case OP_CLEAR:
			inst_init("CLEAR", 0);
			memoryInit(USER_RAM);
			break;
		case OP_MFREE:
			inst_init("MFREE", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			*registers[0].p = (long)mem_isFree(USER_RAM, CPU_Registers->R_MAR);
			break;
		case OP_MEMWRITE:
			inst_init("MEMWRITE", 0);
			writeToMemory(S_NULL, USER_RAM, CPU_Registers->R_MAR, S_NULL, CPU_Registers->R_MDR, NULL);
			break;
		case OP_SMEMWRITE:
			inst_init("SMEMWRITE", 0);
			(CPU_Registers->R_SP)++;
			writeToMemory(S_NULL, USER_RAM, CPU_Registers->R_SP, S_NULL, CPU_Registers->R_MDR, NULL);
			break;
		case OP_MEMLOAD:
			inst_init("MEMLOAD", 1);
			CPU_Registers->R_MDR = (int)readFromMemory(S_NULL, USER_RAM, CPU_Registers->R_MAR, S_NULL, NULL);
			break;
		case OP_SMEMLOAD:
			inst_init("SMEMLOAD", 1);
			CPU_Registers->R_MDR = (int)readFromMemory(S_NULL, USER_RAM, CPU_Registers->R_SP, S_NULL, NULL);
			(CPU_Registers->R_SP)--;
			break;
		//----------------------------------
		// Register Operations
		case OP_REGSET:
			inst_init("REGSET", 2);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (isdigit(*CPU_Registers->R_IR[1]) || *CPU_Registers->R_IR[1] == '-') // Is the value an integer?
				*registers[0].p = atoll(CPU_Registers->R_IR[1]);
			else // Or an ASCII character?
				*registers[0].p = (int)*CPU_Registers->R_IR[1];
			break;
		case OP_REGCOPY:
			inst_init("REGCOPY", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			*registers[1].p = *registers[0].p;
			break;
		case OP_EQ:
			inst_init("EQ", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_EQ(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_HIGHER:
			inst_init("HIGHER", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_HIGHER(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_LOWER:
			inst_init("LOWER", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_LOWER(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_SHIFTFORWARD:
			inst_init("SHIFTFORWARD", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_SHIFTFORWARD(registers[0].p, registers[1].p, registers[2].p);
			break;
		case OP_SHIFTBACK:
			inst_init("SHIFTBACK", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			F_SHIFTBACK(registers[0].p, registers[1].p, registers[2].p);
			break;
		//----------------------------------
		// IO
		case OP_INPUT:
			inst_init("INPUT", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			input_delay_handler();
			*registers[0].p = getchar_fixed();
			input_delay_handler();
			break;
		case OP_OUTPUT:
			inst_init("OUTPUT", 1);
			if (**CPU_Registers->R_IR == 'C') // Cn (Constant Strings)
				registers[0].type = M_CONST;
			else {
				hashRegisters(1, CPU_Registers->R_IR, registers);
				makeRegisterPointers(1, registers);
			}
			if (registers[0].type == M_CONST) {
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
			} else
				putchar(*registers[0].p);
			break;
		case OP_GETKEY:
			inst_init("GETKEY", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			input_delay_handler();
			*(registers[0].p) = ((ch = _getch()) == 0 || ch == 224) ? _getch() : ch; // _getch() returns 0 or 224 for special keys, which is useless for us..
			input_delay_handler();
			break;
		case OP_CLS:
			inst_init("CLS", 0);
			clear_screen();
			break;
		//----------------------------------
		// Program logic
		case OP_GOTO:
			inst_init("GOTO", 1);
			CPU_Registers->R_PC = findGotoPoint(CPU_Registers->R_IR[0]) + 1;
			break;
		case OP_RUN:
			inst_init("RUN", 1);
			inst = hashStr(CPU_Registers->R_IR[0]);
			shift_IR(BACK);
			decode_execute(inst, drive);
			break;
		case OP_CRUN:
			inst_init("CRUN", 2);
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
			inst_init("SHUTDOWN", 0);
			system_shutdown();
			break;
		case OP_HIBERNATE:
			inst_init("HIBERNATE", 0);
			system_hibernate();
			break;
		//----------------------------------
		// Time
		case OP_GETTIME:
			inst_init("GETTIME", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			time((time_t *)registers[0].p);
			break;
		case OP_GETTIMEZONE:
			inst_init("GETTIMEZONE", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			*(time_t *)registers[0].p = get_currentTimeZone_offset();
			break;
		//----------------------------------
		// Definitions
		case OP_CONST:
			inst_init("CONST", 2);
			const_def();
			break;
		case OP_ERR:
			inst_init("ERR", 3);
			error_def();
			break;
		case OP_CERR:
			inst_init("CERR", 3);
			hashRegisters(7, CPU_Registers->R_IR, registers);
			makeRegisterPointers(7, registers);
			print_error(registers);
			break;
		case OP_PROC:
			inst_init("PROC", 1);
			CPU_Registers->R_PC = proc[programsTop - 1][findProc(CPU_Registers->R_IR[0])].end;
			(CPU_Registers->R_PC)++;
			break;
		case OP_RUNPROC:
			inst_init("RUNPROC", 1);
			prochelper[programsTop - 1][proc_stack_top[programsTop - 1]] = CPU_Registers->R_PC;
			proc_stack_top[programsTop - 1]++;
			strcpy(buffer2, CPU_Registers->R_IR[0]);
			CPU_Registers->R_PC = proc[programsTop - 1][findProc(buffer2)].start;
			(CPU_Registers->R_PC)++;
			break;
		case OP_ENDPROC:
			inst_init("ENDPROC", 0);
			CPU_Registers->R_PC = prochelper[programsTop - 1][proc_stack_top[programsTop - 1] - 1];
			proc_stack_top[programsTop - 1]--;
			break;
		case OP_ENABLEECHO:
			inst_init("ENABLEECHO", 0);
			SetConsoleMode(hStdin, echo_on);
			break;
		case OP_DISABLEECHO:
			inst_init("DISABLEECHO", 0);
			SetConsoleMode(hStdin, echo_off);
			break;
		case OP_LOADTOMEMORY:
			inst_init("LOADTOMEMORY", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			loadToMemory(drive, NULL);
			*registers[0].p = (programs + programsTop - 1)->start;
			break;
		case OP_UNLOADFROMMEMORY:
			inst_init("UNLOADFROMMEMORY", 1);
			unloadFromMemory();
			break;
		case OP_CURSORUP:
			inst_init("CURSORUP", 1);
			printf("\x1b[F");
			break;
		case OP_CURSORDOWN:
			inst_init("CURSORDOWN", 1);
			printf("\x1b[E");
			break;
		case OP_DRAWPIXEL:
			inst_init("DRAWPIXEL", 3);
			hashRegisters(3, CPU_Registers->R_IR, registers);
			makeRegisterPointers(3, registers);
			printf("\x1b[48;2;%lld;%lld;%lldm \x1b[0m", *registers[0].p, *registers[1].p, *registers[2].p);
			break;
		case OP_WRITELOG:
			inst_init("WRITELOG", 0);
			fprintf(debugOutput, "%lld\n", CPU_Registers->R_U);
			break;
		case OP_STARTSTREAM:
			inst_init("STARTSTREAM", 0);
			start_stream();
			break;
		case OP_STOPSTREAM:
			inst_init("STOPSTREAM", 0);
			stop_stream();
			break;
		case OP_SETBPM:
			inst_init("SETBPM", 1);
			hashRegisters(1, CPU_Registers->R_IR, registers);
			makeRegisterPointers(1, registers);
			if (registers[0].p)
				setBPM((int)*registers[0].p);
			else
				setBPM(atoi(CPU_Registers->R_IR[0]));
			break;
		case OP_PLAYNOTE:
			inst_init("PLAYNOTE", 2);
			hashRegisters(2, CPU_Registers->R_IR, registers);
			makeRegisterPointers(2, registers);
			play_note((unsigned char)*registers[0].p, (unsigned char)*registers[1].p);
			break;
		case OP_DEBUG_BPOINT:
			inst_init("DEBUG", 0);
			break;
	}
	clock_pulse();
}
void runCPU(FILE *drive, bool hibernate_flag) {
	start = clock(); //Set start time
	if (!hibernate_flag)
		CPU_Registers->R_PC = programs->start; //Set the PC value to the start of the currently running program...
	//CPU Cycle
	unsigned long inst = 0;
	while (CPU_Registers->R_PC < pc_max && !CPU_Registers->restart_trigger && CPU_Registers->power_state) { //FETCH till the program ends...
		inst = fetch();
		decode_execute(inst, drive);
	}
	if (CPU_Registers->R_PC < pc_max) //Wait for the monitor's side to load stuff
		pause_program();
}


