#include <stdio.h>
#include "systemConfiguration.h"

extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];
//Function Declaration
FILE *disk_init();
void disk_close(FILE *);


FILE *disk_init(){
	FILE *drive = fopen("drive.bin", "w+");
	if (!drive) return NULL;
	return drive;
}

void disk_close(FILE *drive){
	fclose(drive);
}
