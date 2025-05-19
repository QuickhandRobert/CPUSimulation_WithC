#define FILENAME_SIZE 32
#define MEMORY_SIZE 256
#define STRING_SIZE 256
#define EMPTY_MEMORY_VALUE 0x7FFFFFFF
#define DISK_SIZE 1024 * 1024 //1MB Disk size
#define SECTOR_SIZE 512 //512 Bytes for each sector
#define NUMBER_OF_FILES DISK_SIZE/SECTOR_SIZE //Number of file sectors
#define LINE_BUFFER_SIZE 128
//#define ADD 1
//#define SUB 2
//#define MUL 3
//#define DIV 4
//#define LO 5
//#define INC 6
//#define DEC 7
//#define NEG 8
//#define SET 10
//#define COPY 11
//#define FLUSH 12
//#define CLEAR 13
//#define CMP 14
//#define BCMP 15
//#define EQ 16
//#define OR 17
//#define AND 18
//#define NOT 19
//#define XOR 20
//#define XNOT 21
//#define NAND 22
//#define NOR 23
struct metadata {
	char filename[FILENAME_SIZE];
	int size;
	int start_sector;
	int isFree;
};