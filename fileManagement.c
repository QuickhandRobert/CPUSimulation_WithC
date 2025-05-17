#include <stdio.h>
#include <string.h>
#include "systemConfiguration.h"
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];
extern struct metadata metadataMemory[DISK_SIZE / SECTOR_SIZE];
extern int numberOfFiles;
//Function Declaration
void openFile(char[], FILE *);


void metadata_init(FILE *drive){
	int start, i;
	char buffer[LINE_BUFFER_SIZE] = {0};
	char secondBuffer[LINE_BUFFER_SIZE] = {0};
	fgets(buffer, LINE_BUFFER_SIZE, drive);
	for (i = 0; strcmp(buffer, "END\r\n") != 0; i++){
		start = 0;
		if (strcmp(buffer, "") == 0) return;
		start = stringCut(secondBuffer, buffer, start, -1, ' ') + 1;
		stringCopy(metadataMemory[i].filename, secondBuffer);
		start = stringCut(secondBuffer, buffer, start, -1, ' ') + 1;
		metadataMemory[i].size = atoi(secondBuffer);
		stringCut(secondBuffer, buffer, start, -1, -1);
		metadataMemory[i].start_sector = atoi(secondBuffer);
		fgets(buffer, LINE_BUFFER_SIZE, drive);
	}
	numberOfFiles = i;
}

long fileSearch (char filename[]){
	long i, found;
	for (i = 0; (found = strcmp(metadataMemory[i].filename, filename)) != 0 && i < NUMBER_OF_FILES; i++);
	if (found == 0) return metadataMemory[i].start_sector;
	return -1;
}

void openFile (char filename[], FILE *drive){
	fpos_t newpos;
	newpos = (long)fileSearch(filename);
	return fsetpos(drive, &newpos);
}
void writeToFile(char text[], FILE *drive){
	fputs(text, drive);
}

void createFile(char filename[]){
	//Memory
	strcpy(metadataMemory[numberOfFiles].filename, filename);
	metadataMemory[numberOfFiles].start_sector = numberOfFiles * SECTOR_SIZE;
	metadataMemory[numberOfFiles].size = 0;
	numberOfFiles++;
}

void updateDriveMetadata(FILE *drive){
	
}
