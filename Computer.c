#include <stdio.h>
#include "systemConfiguration.h"
#include <string.h>
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];

int main() {
//	int value[10] = {'S','A','L'};
//	writeToMemory(1, 14, value);
//	printf("%s", userMemory[14]);
	FILE *drive = fopen("drive.bin", "rb+");
	metadata_init(drive);
	createFile("Kir.exe");
	openFile("kir.exe", drive);
	writeToFile("Salamo Kir", drive);
}