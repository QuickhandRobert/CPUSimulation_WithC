#include <stdio.h>
#include "systemConfiguration.h"
#include <string.h>
extern int systemMemory[MEMORY_SIZE];
extern int userMemory[MEMORY_SIZE][STRING_SIZE];
int main() {
	int value[10];
	value [0] = 255;
	writeToMemory(0, 14, value);
	printf("%d", userMemory[14][0]);
}