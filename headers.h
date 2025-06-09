#define FILENAME_SIZE 32
#define MEMORY_SIZE 256
#define INSTRUCTIONS_NUMBER 256
#define STRING_SIZE 256
#define EMPTY_MEMORY_VALUE 0x7FFFFFFF
#define DISK_SIZE 1024 * 1024 //1MB Disk size
#define SECTOR_SIZE 512 //512 Bytes for each sector
#define NUMBER_OF_FILES DISK_SIZE/SECTOR_SIZE //Number of file sectors
#define LINE_BUFFER_SIZE 128
#define HASH_KEY 5381
#define SYNTAX_LIMIT 7
#define CONSTANT_STRINGS 64
//*****************************************
//Instructions
#define OR 5862598
#define AND 193450424
#define NOT 193464630
#define XOR 193475518
#define NAND 2089350118
#define NOR 193464628
#define FOPEN 220095197
#define MKF 193463395
#define RL 5862691
#define RNC 193468936
#define PF 5862619
#define FEX 193455592
#define FSIZE 220232006
#define ADD 193450094
#define SUB 193470255
#define DIV 193453544
#define MUL 193463731
#define LO 5862496
#define INC 193459135
#define DEC 193453393
#define NEG 193464287
#define COPY 2088970144
#define FLUSH 219993287
#define CLEAR 216417516
#define MEMWRITE 82081455
#define PMEMWRITE 3794864031
#define MEMLOAD 3516152612
#define PMEMLOAD 505048596
#define REGSET 3425631311
#define MARSET 3225605969
#define REGCOPY 1376119422
#define EQ 5862267
#define INPUT 223617557
#define OUTPUT 3327664310
#define CLS 2088966897
#define GOTO 2089114014
#define RUN 193469178
#define CRUN 2088973565
#define SHUTDOWN 2278134369
#define HIBERNATE 2471584439
#define CONST 216535724
#define ERR 193454926
#define CERR 2088959313
#define RUNPROC 2011057550
#define END 2089040454
//*****************************************
//CPU Registers
#define SP 5862728
#define AC 5862121
#define MAR 193463077
#define MDR 193463176
#define X 177661
#define Y 177662
#define Z 177663
#define S 177656
#define A 177638
//*****************************************
//Structs
typedef struct metadata {
	char filename[FILENAME_SIZE];
	int size;
	int start_sector;
	int isFree;
} metadata;
typedef struct systemRAM {
	unsigned long int instruction;
	char data[10][STRING_SIZE];
	int type;
	int isFree;
} systemRAM;
typedef struct userRAM {
	long int data;
	int type;
	int isFree;
} userRAM;