#include <stdio.h>
#include "headers.h"
#include <string.h>
extern systemRAM systemMemory[MEMORY_SIZE];
extern userRAM userMemory [MEMORY_SIZE];
FILE *drive;

int main(int agrc, char *argv[0]) {
	char str[256];
	FILE *drive = fopen("drive.bin", "rb+");
	metadataMemoryInit();
	metadata_init(drive);
	memoryInit(0);
	createFile("Salam.txt", drive);
	openFile("Salam.txt", drive);
	writeToFile("@Test Sequence Initiated", "", drive);
	appendToFile("REGSET S STR \"Salam Dash\"","", drive);
	appendToFile("OUTPUT STR S", "", drive);
	appendToFile("CLS", "", drive);
	appendToFile("GOTO 1", "", drive);
	loadToMemory(drive, "Salam.txt");
	runCPU(drive);
}