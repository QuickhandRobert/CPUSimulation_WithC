#include <stdio.h>
#include "systemConfiguration.h"
#include <string.h>
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];

int main() {
	FILE *drive = fopen("drive.bin", "rb+");
	metadataMemoryInit();
	metadata_init(drive);
}