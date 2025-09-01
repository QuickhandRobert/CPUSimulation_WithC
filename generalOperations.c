#include <stdio.h>
#include <windows.h>
#include "headers.h"
#include <string.h>
#include <stdlib.h>
//Global Variables
static char *olds; //Used by strtok()
/*****************************************************************************************
* Func: strtok_fixed                                                                     *
* Params: char *s: Input String (NULL To Use Previous)                                   *
*         char *delim: Delimeter to Tokenize by                                          *
*         char seperator: Ignore any delimeters inside A couple of this character        *
*                                                                                        *
*         Return: char *: Character pointer to start of the tokenized string             *
*****************************************************************************************/
char *strtok_fixed (char *s, const char *delim, const char seperator) {
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
/*********************************************************************************************
* Func: fgets_fixed                                                                          *
* Params: char *buf: Where to write the string to                                            *
*         const int n: Maximum Number of Characters                                          *
*         FILE *fp: Pointer to drive.bin                                                     *
*         char seperator: Ignore newline characters (\n) inside a couble of this chacter     *
*                                                                                            *
* Return: Char pointer to start of the output string                                         *
*********************************************************************************************/
char *fgets_fixed(char *buf, int n, FILE *fp, char separator, bool resetPos) {
	if (n <= 1 || !fp) return NULL;
	int i, c;
	fpos_t cur;
	fgetpos(fp, &cur);
	bool in_delimiter = false;
	if (buf)
		*buf = '\0';
	for (i = 0; i < n - 1; i++) {
		c = fgetc(fp);
		if (c == EOF) {
			if (i == 0) return NULL;
			break;
		}
		if (buf) {
			buf[i] = (char)c;
			buf[i + 1] = '\0';
		}
		if (c == separator)
			in_delimiter = !in_delimiter;
		else if (c == '\n' && !in_delimiter)
			break;
	}
	if (resetPos)
		fsetpos(fp, &cur);
	return buf;
}
/*****************************************************************
* Func: strcut                                                   *
* Params: char *dest: Destination                                *
*         char *source: Source                                   *
*         const int start: Starting Point (-1 For beginning)     *
*         const int end: Ending Point (-1 for end of string)     *
*                                                                *
*         Return: Char pointer to the output string              *
*****************************************************************/
char *strcut(char *dest, char *source, const int start, const int end) {
	char *s = start == -1 ? dest : (source + start);
	int s_end = end == -1 ? strlen(source) : end;
	strncpy(dest, s, s_end - start + 1);
}
/********************************************
* Func: isStringEmpty                       *
* Params: const char *str: Input String     *
*                                           *
* Return: True or False                     *
********************************************/
bool isStringEmpty(const char *str) {
	if (str)
		if (*str == '\0')
			return true;
	return false;
}
/********************************************
* Func: isLineEmpty                         *
* Params: const char *str: Input String     *
*                                           *
* Return: True or False                     *
********************************************/
bool isLineEmpty(const char *str) {
	if (*str == '\n')
		return true;
	return false;
}
/********************************************
* Func: hashStr                             *
* Params: char *str: Input String           *
*                                           *
* Return: Hashed String                     *
********************************************/
unsigned long hashStr(char *str) {
	unsigned long hash = HASH_KEY;
	char *c;
	for (c = str; *c != '\0'; c++)
		hash = ((hash << 5) + hash) + *c;
//	printf("----------------------------------\n");
//	printf("%u\n", hash);
	return hash;
}
/****************************************************************
* Func: hashRegisters                                           *
* Params: const int n: Total Number of Registers                *
*         char **buffer: Current Instruction Register Value     *
*         registerP *registers: List of Registers (Array)       *
*                                                               *
* Return: none                                                  *
****************************************************************/
void hashRegisters(const int n, char buffer[SYNTAX_LIMIT][STRING_SIZE], registerP_t *registers) {
	char *res;
	registerP_t tmp;
	char buff_i[STRING_SIZE];
	for (int i = 0; i < n; i++) {
		strcpy(buff_i, buffer[i]); //Backup original string, strtok tends to fuck the original one up...
		if (res = strstr(buff_i, INDEX_SEPERATOR)) {
			*res = '\0';
			registers[i].hashed = hashStr(buff_i);
			res++;
			if (isdigit((int)*res))
				registers[i].index = atoi(res);
			else {
				tmp.hashed = hashStr(res);
				tmp.index = 0;
				makeRegisterPointers(1, &tmp);
				registers[i].index = *(int *)tmp.p;
			}
		} else {
			registers[i].hashed = hashStr(buff_i);
			registers[i].index = 0;
		}


	}
}
/********************************************
* Func: removeNewLine                       *
* Params: char *str: Input String           *
*                                           *
* Return: none                              *
********************************************/
void removeNewLine(char *str) {
	char *c = (str + strlen(str) - 1);
	*c = *c == '\n' ? '\0' : *c;
}
/**********************************************************************
* Func: humanSize                                                     *
* Params: long long int bytes: Size in bytes                          *
*                                                                     *
* Return: char * to Humanized size string                             *
* Desc: Humanizes a given amount of bytes for easier understanding,   *
*       Using suffixes such as KB, MB, GB, etc                        *
* Note: not my code, credits to dgoguerra                             *
**********************************************************************/
char *humanSize(long long int bytes) {
	char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
	char length = sizeof(suffix) / sizeof(suffix[0]);
	int i = 0;
	double dblBytes = bytes;

	if (bytes > 1024) {
		for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
			dblBytes = bytes / 1024.0;
	}

	static char output[200];
	sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
	return output;
}
/************************************************************************
* Func: fgets_lineByLine                                                *
* Params: FILE *f                                                       *
*         char buffer[][STRING_SIZE]: Destination buffer                *
*                                                                       *
* Return: Number of read lines                                          *
* Desc: Reads a file line by line and outputs them to a given buffer    *
************************************************************************/
int fgets_lineByLine(FILE *f, char buffer [][STRING_SIZE]) {
	fpos_t f_zero = 0;
	int i;
	fsetpos(f, &f_zero);
	for (i = 0; !feof(f); i++) {
		fgets(buffer[i], STRING_SIZE, f);
		removeNewLine(buffer[i]);
	}
	fsetpos(f, &f_zero);
	return --i;
}
/************************************************************************
* Func: fputs_lineByLine                                                *
* Params: FILE *f                                                       *
*         const char buffer[][STRING_SIZE]: Destination buffer          *
*         const int cnt: Number of total line                           *
* Return: None                                                          *
* Desc: Same as previous one, but fputs                                 *
************************************************************************/
void fputs_lineByLine(FILE *f, const char buffer [][STRING_SIZE], const int cnt) {
	fpos_t f_zero = 0;
	int i;
	fsetpos(f, &f_zero);
	for (i = 0; i < cnt; i++) {
		fputs(buffer[i], f);
		fputc('\n', f);
	}
	fsetpos(f, &f_zero);
}
/********************************************
* Func: switch_strings                      *
* Params: char *a                           *
*         char *                            *
*                                           *
* Return: none                              *
* Desc: Switches two strings places         *
********************************************/
void switch_strings(char *a, char *b) {
	char temp[STRING_SIZE];
	memcpy(temp, b, STRING_SIZE);
	memcpy(b, a, STRING_SIZE);
	memcpy(a, temp, STRING_SIZE);
}
void shift_strings(char dest[][STRING_SIZE], int index, int cnt, int direction) {
	int a, b;
	switch (direction) {
		case BACK:
			if (index == 0) {
				a = 0;
				b = cnt - 1;
			} else {
				a = index;
				b = index - 1;
			}
			break;
		case FORWARD:
			if (index == cnt - 1) {
				a = index;
				b = 0;
			} else {
				a = index;
				b = index + 1;
			}
			break;
	}
	switch_strings(dest[a], dest[b]);
}
/********************************************
* Func: getchar_fixed                       *
* Params: none                              *
*                                           *
* Return: Read character                    *
* Desc: getchar, but returns zero on \n     *
********************************************/
int getchar_fixed() {
	int c = getchar();
	if (c == '\n')
		return 0;
	return c;
}
/******************************************************************
* Func: intPtoString                                              *
* Params: char *char_p: Character array (dest or src)             *
*         long long *int_p: Integer array (dest or src)           *
*         const bool direction: (INTP_TO_CHARP, CHARP_TO_INTP)    *
*         const int max_char                                      *
*                                                                 *
*         Return: none                                            *
******************************************************************/
void intPtoString(char *char_p, long long int *int_p, const bool direction, const int max_char) {
	long long int *i;
	char *c;
	switch (direction) {
		case INTP_TO_CHARP:
			for (i = int_p; *i != 0 && (i - int_p) < max_char; i++)
				char_p[i - int_p] = *i;
			char_p[i - int_p] = '\0';
			break;
		case CHARP_TO_INTP:
			for (c = char_p; *c != '\0' && c - char_p < max_char; c++)
				int_p[c - char_p] = *c;
			int_p[c - char_p] = 0;
			break;
	}
}
/********************************************
* Func: pause_program                       *
* Params: none                              *
*                                           *
* Return: none                              *
********************************************/
void pause_program(){
	while (true)
		Sleep(WATCH_FOR_POWEROFF_WAIT_INTERVAL);
}
