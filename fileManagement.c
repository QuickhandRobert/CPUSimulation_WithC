#include <stdio.h>
#include <string.h>
#include "systemConfiguration.h"
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];
extern struct metadata metadataMemory[DISK_SIZE / SECTOR_SIZE];
extern int freeSectors[NUMBER_OF_FILES];
//Function Declaration
void openFile(char[], FILE *);

void sector_init(int start, int size, int status){
	int to = start + size;
	for (int i = start; i < to; i++) freeSectors[i] = status; 
}

void metadata_init(FILE *drive){
	int start, i;
	char buffer[LINE_BUFFER_SIZE] = {0};
	char secondBuffer[LINE_BUFFER_SIZE] = {0};
	freeSectors[0] = 1; //First sector is preserved for the metadata
	fgets(buffer, LINE_BUFFER_SIZE, drive);
	for (i = 0; strcmp(buffer, "END\n") != 0 && strcmp(buffer,"END") != 0; i++){
		start = 0;
		if (strcmp(buffer, "") == 0) return;
		start = stringCut(secondBuffer, buffer, start, -1, ' ') + 1;
		stringCopy(metadataMemory[i].filename, secondBuffer);
		start = stringCut(secondBuffer, buffer, start, -1, ' ') + 1;
		metadataMemory[i].size = atoi(secondBuffer);
		stringCut(secondBuffer, buffer, start, -1, -1);
		metadataMemory[i].start_sector = atoi(secondBuffer);
		sector_init(metadataMemory[i].start_sector, metadataMemory[i].size, 1);
		fgets(buffer, LINE_BUFFER_SIZE, drive);
		metadataMemory[i].isFree = 0;
	}
}
int sectorSearch(){
	int i;
	for (i = 0; freeSectors[i]; i++);
	return i;
}
int metadataMemorySearch(){
	int i;
	for (i = 0; metadataMemory[i].isFree == 0; i++);
	return i;
}
long fileSearch (char filename[]){
	long i, found;
	for (i = 0; (found = strcmp(metadataMemory[i].filename, filename)) != 0 && i < NUMBER_OF_FILES; i++);
	if (found == 0) return i	;
	return -1;
}
void openFile (char filename[], FILE *drive){
	fpos_t newpos;
	newpos = (long)metadataMemory[fileSearch(filename)].start_sector * SECTOR_SIZE;
	fsetpos(drive, &newpos);
}
void writeToFile(char text[], char filename[], FILE *drive){
	if (!isStringEmpty(filename)) openFile (filename, drive);
	fpos_t pos;
	fgetpos(drive, &pos);
	fputs(text, drive);
	fputs("\nEND\n", drive);
	fsetpos(drive, &pos); //Reset File position
}
void appendToFile(char text[], char filename[], FILE *drive){
	if (!isStringEmpty(filename)) openFile (filename, drive);
	fpos_t pos;
	fgetpos(drive, &pos);
	char buffer[STRING_SIZE];
	fgets(buffer, STRING_SIZE, drive);
	while (strcmp(buffer, "END\n") != 0 && strcmp(buffer,"END") != 0)	{
		fgets(buffer, STRING_SIZE, drive);
	}
	fseek(drive, -4, SEEK_CUR);
	fputs(text, drive);
	fputs("\nEND\n",drive);
	fsetpos(drive, &pos); //Reset file position
}
void updateDriveMetadata(FILE *drive){
	fpos_t zero = (long)0;
	fsetpos(drive, &zero);
	for (int i = 0; metadataMemory[i].isFree == 0; i++){
		//There's room for optimization here ...
		fprintf(drive, "%s %d %d\n", metadataMemory[i].filename, metadataMemory[i].size, metadataMemory[i].start_sector);
	}
	fputs("END\n", drive);
}
void createFile(char filename[], FILE *drive){
	int memorySlot = metadataMemorySearch();
	int freeSector = sectorSearch();
	strcpy(metadataMemory[memorySlot].filename, filename);
	metadataMemory[memorySlot].start_sector = freeSector;
	metadataMemory[memorySlot].size = 1;
	metadataMemory[memorySlot].isFree = 0;
	updateDriveMetadata(drive);
	sector_init(freeSector, 1, 1);
}
void deleteFile(char filename[], FILE *drive){
	int memorySlot = fileSearch(filename);
	metadataMemory[memorySlot].isFree = 1;
	updateDriveMetadata(drive);
	sector_init(metadataMemory[memorySlot].start_sector, metadataMemory[memorySlot].size, 0);
}
void readLine(char buffer[], char filename[], int resetPos, FILE *drive){
	fpos_t pos;
	if (!isStringEmpty(filename)) openFile (filename, drive);
	fgets(buffer, LINE_BUFFER_SIZE, drive);
	if (resetPos){
		fgetpos(drive, &pos);
		pos /= SECTOR_SIZE;
		pos *= SECTOR_SIZE;
		fsetpos(drive, &pos);
	}
}
char readChar (char filename[], int n, FILE *drive){
	if (!isStringEmpty(filename)) openFile (filename, drive);
	fpos_t pos, newpos;
	char buffer;
	fgetpos(drive, &pos);
	newpos = (long)pos + (long)n;
	fsetpos(drive, &newpos);
	buffer = fgetc(drive);
	fsetpos(drive, &pos);
	return buffer;
}



