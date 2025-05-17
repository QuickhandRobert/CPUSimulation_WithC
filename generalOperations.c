#include <stdio.h>
#include "systemConfiguration.h"
//Function Declaration
int stringLength(int []);
void stringCopy(int[], int[]);
//int stringCut(int [], int[], int, int, int);
int stringLength(int s1[]) {
	int i;
	for (i = 0; s1[i] != '\0'; i++);
	return i;
}
void stringCopy(int s1[], int s2[]) {
	int length = stringLength(s2);
	for (int i = 0; i < STRING_SIZE && i < length; i++) s1[i] = s2[i];
	s1[length] = '\0';
}

int stringCut(char s1[], char s2[], int start, int end, int seperator) {
	int length = stringLength(s2);

	int i;
	if (start == -1) start = 0;
	if (end == -1)
		if (seperator == -1) for (i = start; i < length; i++) s1[i - start] = s2[i];
		else for (i = start; s2[i] != seperator; i++){
			s1[i - start] = s2[i];
		} 
	else for(i = start; i < end; i++) s1[i - start] = s2[i];

	s1[i] = '\0';
	return i;
}