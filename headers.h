#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#ifndef _HEADERS_H_
#define _HEADERS_H_
//Bios configurations
#define NUMBER_OF_TIMEZONES 38
#define MAX_DRIVES 64
#define POST_WAIT_DURATION 3000 //The amount of time the BIOS waits before booting, after POST, in ms
#define DRIVE_EXT ".bin"
#define DRIVES_DIR "./"
#define BOOTDEV_CFG_FILENAME ".bootdev"
#define TIMEZONE_CFG_FILENAME ".timezone"
#define SYSTEM_RESTART_TRIGGER "system_restart" //The command line attribute which the systems looks for to know if it has be restarted or a fresh boot
#define WATCH_FOR_POWEROFF_WAIT_INTERVAL 10 //How often should the system check for the power_state (changed by monitor.exe), in ms
//Frequently used values
#define STRING_SIZE 256
#define REG_STRING_SIZE 2 * STRING_SIZE
#define LINE_BUFFER_SIZE 128
#define HASH_KEY 5381 //Changnig this requires you to manually set the instructions macros again
#define DOTS_MAX 64 //Used in print_dots(), sanity checks
#define DOTS_DURATION_MAX 20000 //Used in print_dots(), sanity checks
//Command parser configuations
#define MAX_PROGRAMS 16
#define SYNTAX_LIMIT 8
#define CONSTANT_STRINGS 128 //Number of max CONST definitions
#define ERRORS 64 //Number of max ERR definitions
#define MAX_PROCS 128
#define MAX_GOTO_POINTS 128
#define GOTO_POINT_INDICATOR ':'
#define COMMENT_SIGN '@'
#define ERR_INT_INDICATOR '#'
#define ERR_STR_INDICATOR '$'
#define INDEX_SEPERATOR ":"
#define GOTO_POINT 255255
#define END 2089040454
#define ENDN 193454780
//Hard drive attributes
#define FILENAME_SIZE 32
#define DEFAULT_DRIVE_SIZE 1024 * 1024 //Values used if not provided by the drive itself 
#define DEFAULT_SECTOR_SIZE 512 //Values used if not provided by the drive itself 
//Memory attributes
#define MEMORY_SIZE 8192
//CPU Clock configurations
#define MINIMUM_CLOCK_DURATION 0 //Used for error handling, prevents the system from running too fast
#define CLOCK_PULSE 1
#define CLOCK_PULSE_TOGGLE_RATIO 50 //What percentage of the clock pulse duration should the monitor's clockpulse indicator turn on?
//Audio Stuff
#define AUDIO_AMPLITUDE INT_MAX / 2
#define AUDIO_SAMPLE_RATE 44100
#define DEFAULT_BPM 120
#define AUDIO_BUFFER_SIZE AUDIO_SAMPLE_RATE * 3
//-------------------------------------------
//Definitions
#define SYSTEM_RAM 0
#define USER_RAM 1
#define BACK 0
#define FORWARD 1
#define M_INT 0
#define M_STR 1
#define M_CONST 2
#define FETCH 0
#define DECODE 1
#define EXECUTE 2
#define REG_INIT 0
#define REG_UNINIT 1
#define S_NULL 0
#define FILE_FAILED -1
#define NOT_FOUND -1
#define LINE_COMMENT -1
#define LINE_OK 0
#define PROGRAM_ENDED -2
#define CHARP_TO_INTP 0
#define INTP_TO_CHARP 1
//-------------------------------------------
#endif
//Structs
typedef struct metadata {
	char filename[FILENAME_SIZE];
	int size;
	long start_sector;
	bool isFree;
} metadata;
typedef struct systemRAM {
	unsigned long instruction;
	char data[SYNTAX_LIMIT][STRING_SIZE];
	int type;
	bool isFree;
} systemRAM;
typedef struct userRAM {
	int data;
	bool isFree;
} userRAM;
typedef struct registerP {
	long long *p;
	int type;
	int index;
	unsigned long hashed;
} registerP;
typedef struct procedure {
	char name[LINE_BUFFER_SIZE];
	int start;
	int end;
} procedure;
	struct R_IR_INSTRUCTION{
		char inst_name[CONSTANT_STRINGS];
		int inst_params;
	};
typedef struct CPU_registers {
	char R_IR[SYNTAX_LIMIT][STRING_SIZE];
	long long int R_PC;
	long long int R_SP; //Stack Pointer
	long long int R_AC; //Accumulator
	long long int R_MAR; //Memory Address
	long long int R_MDR; //Value
	long long int R_U; //General Purpose
	long long int R_V; //General Purpose
	long long int R_X; //General Purpose (Unsigned)
	long long int R_Y; //General Purpose
	long long int R_Z; //General Purpose
	long long int R_C; //Clock accumalator
	struct R_IR_INSTRUCTION R_IR_INST;
	long long R_S[REG_STRING_SIZE]; //String Handling
	long long R_A[REG_STRING_SIZE]; //String Handling
	char CONSTSTR[CONSTANT_STRINGS][STRING_SIZE];
	int step;
	bool cp_toggle;
	bool cp_differ_toggle;
	int CPU_Clock;
	volatile bool power_state;
	volatile bool restart_trigger;
} CPU_registers;
typedef struct disk_info {
	long totalSize;
	long sectorSize;
	long disk_info_len;
} disk_info;
typedef struct gotoPoint {
	long pos;
	char name[STRING_SIZE];
} gotoPoint;
typedef struct time_zone {
	time_t difference;
	char desc[STRING_SIZE];
} time_zone;
typedef struct program {
	int start;
} program;
//-------------------------------------------
//Enums
enum post_stat {
	SETUP, BOOT_MENU, BOOT_OK
};
enum keys {
	F11 = 133, F12 = 134, INSERT = 82, DEL = 83, PGDN = 81, PGUP = 73, DOWNARROW = 80, UPARROW = 72 
};
enum errors_def {
	OPEN_DIR_FAILED,
	OPEN_DRIVE_FAILED,
	BOOTDRIVE_NOT_FOUND,
	INVALID_INPUT_NOT_DIGITS,
	PRINT_DOTS_MAXIMUM_EXCEEDED,
	OPENING_BOOTDEV_FAILED,
	OPENING_TIMEZONECFG_FAILED,
	FILE_EMPTY,
	FILEINIT_FAILURE_FILENAME_NOTPRESENT,
	FILEINIT_FAILURE_SIZE_INVALID,
	FILEINIT_FAILURE_SECTOR_INVALID,
	FILE_NOT_FOUND,
	FILE_FULL,
	DISK_FULL,
	FILE_ALREADY_EXISTS,
	FILE_DOESNT_EXISTS,
	WRITE_FAILED_ACCESS_DENIED,
	DRIVE_SIZE_INVALID,
	SECTOR_SIZE_INVALID,
	MEMORY_LIMIT_EXCEEDED,
	CPU_LIMIT_EXCEEDED,
	ERROR_DEFINITION_FAILED_INVALID_INDEX,
	CONST_DEFINITION_FAILED_INVALID_INDEX,
	ERROR_CALL_FAILED_INVALID_INDEX
};
enum operations_hashed {
    OP_OR              = 5862598,
    OP_AND             = 193450424,
    OP_NOT             = 193464630,
    OP_XOR             = 193475518,
    OP_NAND            = 2089350118,
    OP_BITAND          = 2804656567,
    OP_BITOR           = 215140581,
    OP_NOR             = 193464628,
    OP_FOPEN           = 220095197,
    OP_RM              = 5862692,
    OP_MKF             = 193463395,
    OP_RENAME          = 3425863517,
    OP_RNC             = 193468936,
    OP_WNC             = 193474381,
    OP_FEX             = 193455592,
    OP_FSIZE           = 220232006,
	OP_GETFILEINFO     = 2071711505,
    OP_ADD             = 193450094,
    OP_SUB             = 193470255,
    OP_DIV             = 193453544,
    OP_MUL             = 193463731,
    OP_LO              = 5862496,
    OP_INC             = 193459135,
    OP_DEC             = 193453393,
    OP_NEG             = 193464287,
    OP_COPY            = 2088970144,
    OP_FLUSH           = 219993287,
    OP_CLEAR           = 216417516,
    OP_MFREE           = 228075380,
    OP_MEMWRITE        = 82081455,
    OP_PMEMWRITE       = 3794864031,
    OP_MEMLOAD         = 3516152612,
    OP_PMEMLOAD        = 505048596,
    OP_REGSET          = 3425631311,
    OP_REGCOPY         = 1376119422,
    OP_EQ              = 5862267,
    OP_HIGHER          = 3039009084,
    OP_LOWER           = 227218350,
    OP_SHIFTFORWARD    = 183630808,
    OP_SHIFTBACK       = 3316831508,
    OP_INPUT           = 223617557,
    OP_OUTPUT          = 3327664310,
    OP_GETKEY          = 2995600462,
    OP_CLS             = 193452551,
    OP_GOTO            = 2089114014,
    OP_RUN             = 193469178,
    OP_CRUN            = 2088973565,
    OP_SHUTDOWN        = 2278134369,
    OP_HIBERNATE       = 2471584439,
    OP_GETTIME         = 70894900,
    OP_GETTIMEZONE     = 1769206704,
    OP_CONST           = 216535724,
    OP_ERR             = 193454926,
    OP_CERR            = 2088959313,
    OP_PROC            = 2089440537,
    OP_RUNPROC         = 2011057550,
    OP_ENDPROC         = 2116036176,
    OP_END             = 2089040454,
    OP_ENDN            = 193454780,
    OP_ENABLEECHO      = 1442564523,
    OP_DISABLEECHO     = 3471855896,
    OP_LOADTOMEMORY    = 4290202305,
    OP_UNLOADFROMMEMORY= 2919409973,
    OP_CURSORUP        = 2773867624,
    OP_CURSORDOWN      = 1379224379,
    OP_DRAWPIXEL       = 3968608821,
    OP_WRITELOG        = 513399922,
    OP_STARTSTREAM     = 3329578815,
    OP_STOPSTREAM      = 2083392215,
    OP_SETBPM          = 3465215728,
    OP_PLAYNOTE        = 3366309905,
    OP_DEBUG_BPOINT    = 217349260
};
enum registers_hashed {
	PC  = 5862616,
	SP  = 5862728,
	AC  = 5862121,
	MAR = 193463077,
	MDR = 193463176,
	U   = 177658,
	V   = 177659,
	X   = 177661,
	Y   = 177662,
	Z   = 177663,
	S   = 177656,
	A   = 177638,
	C   = 177640
};
//-------------------------------------------
//Function Declarations:
char *fgets_fixed(char *, int, FILE *, char, bool);
char *strtok_fixed(char *, const char *, const char);
bool isStringEmpty(const char *);
void sector_init(const int, const int, const int);
void removeNewLine(char *);
char *strcut(char *, char *, const int, const int);
bool isStringEmpty(const char *);
bool isLineEmpty(const char *);
unsigned long hashStr(char *);
int read_int(char);
void hashRegisters(const int, char [SYNTAX_LIMIT][STRING_SIZE], registerP*);
void metadata_init(FILE *);
long sectorSearch();
int metadataMemorySearch();
long fileSearch(const char *);
void openFile(const char *, FILE *);
void writeToFile(const char *, const char *, FILE *);
void appendToFile(const char *, char *, FILE *);
void updateDriveMetadata(FILE *);
void createFile(const char *, FILE *);
void renameFile(const char*, const char *, FILE *);
void deleteFile(const char *, FILE *);
void readLine(char *, const char *, const bool, FILE *);
unsigned char readChar(const char *, const int, FILE *);
void writeChar (const char *, const char, const int, FILE *drive);
void metadataMemoryInit();
void memoryInit(int);
void flushMemoryAddress(int, int);
void writeToMemory(const int, const int, const int, const int, const unsigned long, char *);
unsigned long readFromMemory(int, int, int, int, char *);
unsigned long fetch();
void makeRegisterPointers(int, registerP *);
void error_def();
void print_error(registerP *);
void loadToMemory(FILE *, char *);
bool mem_isFree(const int, const int);
void proc_init();
int findProc(const char *);
int decodeInstruction(char *, int, FILE *);
void shift_IR(bool);
void decode_execute(unsigned long, FILE *);
void runCPU(FILE *);
void F_OR(long long *r1, long long *r2, long long *r3);
void F_AND(long long *r1, long long *r2, long long *r3);
void F_NOT(long long *r1);
void F_XOR(long long *r1, long long *r2, long long *r3);
void F_NAND(long long *r1, long long *r2, long long *r3);
void F_NOR(long long *r1, long long *r2, long long *r3);
void F_ADD(long long *r1, long long *r2, long long *r3);
void F_SUB(long long *r1, long long *r2, long long *r3);
void F_DIV(long long *r1, long long *r2, long long *r3);
void F_MUL(long long *r1, long long *r2, long long *r3);
void F_LO(long long *r1, long long *r2, long long *r3);
void F_INC(long long *r1);
void F_DEC(long long *r1);
void F_NEG(long long *r1);
void F_EQ(long long *r1, long long *r2, long long *r3);
void F_LOWER (long long *r1, long long *r2, long long *r3);
void F_HIGHER (long long *r1, long long *r2, long long *r3);
void F_BITAND (long long *r1, long long *r2, long long *r3);
void F_BITOR (long long *r1, long long *r2, long long *r3);
void F_SHIFTBACK (long long *r1, long long *r2, long long *r3);
void F_SHIFTFORWARD (long long *r1, long long *r2, long long *r3);
void shared_memory_handler(const int, const bool);
void clock_pulse(void);
void const_def(void);
char *humanSize(long long int);
void print_dots(const int, const long);
int fgets_lineByLine(FILE* , char [][STRING_SIZE]);
void shift_strings(char [][STRING_SIZE], int, int, int);
void fputs_lineByLine(FILE*, const char [][STRING_SIZE], const int);
disk_info get_disk_info(const char *);
FILE *drive_init(const char *);
void drive_uninit(FILE *);
void system_shutdown();
int getchar_fixed();
char *IR_tokenizer(char *, bool);
void goto_point_init();
void p_error(bool);
void wait_for_power(bool);
unsigned __stdcall watch_for_poweroff(void *);
void intPtoString(char *, long long int *, const bool, const int);//void clear_screen(void);
time_t get_currentTimeZone_offset();
void getFileInfo(long long int*, const int, const int);
bool isSystemOn();
void system_restart();
void squareWaveGenerate(int*, int, int);
void audio_init();
void audio_shutdown();
void start_stream();
void stop_stream();
void setBPM (const int);
void play_note(unsigned char, unsigned char);