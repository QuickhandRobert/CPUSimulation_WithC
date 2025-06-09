#include <stdio.h>
#include "headers.h"
#include <string.h>
//Function Declaration
int stringLength(int []);
void stringCopy(int[], int[]);
//int stringCut(int [], int[], int, int, int);
//Global Variables
static char *olds;
int stringLength(int s1[]) {
	int i;
	for (i = 0; s1[i] != '\0'; i++);
	return i;
}

//int lineEnd(char *str){
//	char c;
//	for (c = str; *c != ' ' && *c != '\n'; c++)
//		;
//	if (c == '\n') return 1;
//	return 0;
//}
void stringCopy(int s1[], int s2[]) {
	int length = stringLength(s2);
	for (int i = 0; i < STRING_SIZE && i < length; i++) s1[i] = s2[i];
	s1[length] = '\0';
}
char *strtok_fixed (char *s, char *delim, char seperator) {
	char *token;
	if (s == NULL)
		s = olds;
	s += strspn (s, delim);
	if (*s == '\0') {
		olds = s;
		return NULL;
	}
	token = s;
	s = strpbrk (token, delim);
	if (s == NULL) {
		olds = memchr (token, '\0', MEMORY_SIZE);
	} else {
		if (*token == seperator) {
			token++;
			while (*s != seperator) s++;
		}
		*s = '\0';
		olds = s + 1;
	}
	return token;
}
int stringCut(char s1[], char s2[], int start, int end, int seperator) {
	int length = strlen(s2);

	int i;
	if (start == -1) start = 0;
	if (end == -1)
		if (seperator == -1) for (i = start; i < length; i++) s1[i - start] = s2[i];
		else for (i = start; s2[i] != seperator; i++) {
				s1[i - start] = s2[i];
			} else for(i = start; i < end; i++) s1[i - start] = s2[i];

	s1[i] = '\0';
	return i;
}
int isStringEmpty(char str[]) {
	if (str[0] == '\0') return 1;
	return 0;
}
int isLineEmpty(char str[]) {
	if (str[0] == '\n') return 1;
	return 0;
}
unsigned long hashStr(char *str) {
	unsigned long hash = HASH_KEY;
	char *c;
	for (c = str; *c != '\0'; c++)
		hash = ((hash << 5) + hash) + *c;
	return hash;
}
void hashRegisters(int n, char buffer[10][STRING_SIZE], unsigned long int *registers) {
	for (int i = 0; i < n; i++) {
		registers[i] = hashStr(buffer[i]);
	}
}
void removeNewLine(char *str) {
	char *c;
	for (c = str; *c != '\n' && *c != '\0'; c++);
	*c = '\0';
}