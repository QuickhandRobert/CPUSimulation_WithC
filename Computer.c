#include <stdio.h>
#include "systemConfiguration.h"
extern int systemMemory[MEMORY_SIZE][STRING_SIZE];
int main() {
	clearMemory();
	printf("%d", systemMemory[10][10]);
}