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
	appendToFile("REGSET S Fucking","", drive);
	appendToFile("REGSET X 2","", drive);
	appendToFile("ERR E0 \"This is a # Test Error Message\nNot # # Test Messages\"","", drive);
	appendToFile("CERR E0 S X S","", drive);
//	appendToFile("REGSET S \"This is a Integer Input Test.\nPlease Enter a Number:\n\"","", drive);
//	appendToFile("OUTPUT S","", drive);
//	appendToFile("INPUT X", "", drive);
//	appendToFile("REGSET S \"The Number You Entered Is:\"", "", drive);
//	appendToFile("OUTPUT S", "", drive);
//	appendToFile("OUTPUT X", "", drive);
//	appendToFile("REGSET S \"\nThis is a String Input Test.\nPlease Enter a String:\"","", drive);
//	appendToFile("OUTPUT S","", drive);
//	appendToFile("INPUT S", "", drive);
//	appendToFile("REGSET A \"The String You Entered Is:\"", "", drive);
//	appendToFile("OUTPUT A", "", drive);
//	appendToFile("OUTPUT S", "", drive);
	loadToMemory(drive, "Salam.txt");
	runCPU(drive);
}