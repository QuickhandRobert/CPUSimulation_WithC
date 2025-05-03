#include <stdio.h>
#include "systemConfiguration.h"
//Function Declaration
int stringLength(int []);
void stringCopy(int[], int[]);

int stringLength(int s1[]){
	int i;
	for (i = 0; s1[i] != '\0'; i++);
	return i;
}
void stringCopy(int s1[], int s2[]){
	int length = stringLength(s1);
	for (int i = 0; i < STRING_SIZE && i < length; i++) s2[i] = s1[i];
}