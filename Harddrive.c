#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "headers.h"
//Memory Variables
static metadata *metadataMemory;
static disk_info *drivemetadata;
//Hard Drive Free Sectors
static bool *freeSectors;
//Error handler
extern enum errors_def error_code;
extern char error_buff[STRING_SIZE];
/***********************************************************
* Func: drive_init                                         *
* Params: const char *filename                             *
*                                                          *
* Return: FILE * to drive file (with the os file opened)   *
***********************************************************/
FILE *drive_init(const char *filename) {
	FILE *drive;
	static disk_info tmp;
	tmp = get_disk_info(filename); //Get drive metadata from the .bin file
	//Assign them to drivemetadata struct
	drivemetadata = &tmp;
	long numberOfFiles = (drivemetadata->totalSize / drivemetadata->sectorSize);
	drive = fopen(filename, "rb+");
	//Memory allocations (dynamically, according to the drive metadata that we just got)/
	metadataMemory = (metadata *)malloc(sizeof(metadata) * numberOfFiles);
	freeSectors = (bool *)malloc(sizeof(bool) * numberOfFiles);
	//Initialize stuff
	sector_init(0, numberOfFiles, 0);
	metadataMemoryInit();
	metadata_init(drive);
	openFile(metadataMemory[0].filename, drive); //Open OS file (first file)
	return drive;
}
/************************************
* Func: drive_uninit                *
* Params: FILE *drive               *
*                                   *
* Return: none                      *
* Desc: Shutdowns stuff             *
************************************/
void drive_uninit(FILE *drive) {
	free(metadataMemory);
	free(freeSectors);
	fclose(drive);
	return;
}
/*******************************************************************************
* Func: sector_init                                                            *
* Param: const int start: Starting Sector                                      *
*        const int size: Size of the current file							   *
*        const int status: Free or not Free?								   *
*																			   *
* Return: none                                                                 *
* Desc: Initializes a file's hdd sectors, to be either free or not free        *
*******************************************************************************/
void sector_init(const int start, const int size, const int status) {
	int to = start + size;
	for (int i = start; i < to; i++)
		freeSectors[i] = status;
}
/*******************************************************************************
* Func: metadata_init                                                          *
* Param: FILE *drive: .bin File pointer                                        *
*                                                                              *
* Return: none                                                                 *
* Desc: Loads the metadata sector from the .bin drive file to metadataMemory   *
*******************************************************************************/
void metadata_init(FILE *drive) {
	int start, i, i_buffer;
	char buffer[LINE_BUFFER_SIZE] = {0}, buffer_backup[LINE_BUFFER_SIZE];
	char *res;
	freeSectors[0] = 1; //First sector is preserved for the metadata
	if (!fgets_fixed(buffer, LINE_BUFFER_SIZE, drive, '\"', false)) { //Skip metadata (first line)
		error_code = FILE_EMPTY;
		p_error(true);
	}
	if (!fgets_fixed(buffer, LINE_BUFFER_SIZE, drive, '\"', false)) {
		error_code = FILE_EMPTY;
		p_error(true);
	}
	strncpy(buffer_backup, buffer, LINE_BUFFER_SIZE);
	removeNewLine(buffer);
	for (i = 0; strcmp(buffer, "END\n") != 0 && strcmp(buffer,"END") != 0; i++) {
		if (strcmp(buffer, "") == 0) {
			error_code = FILEINIT_FAILURE_FILENAME_NOTPRESENT;
			strcpy(error_buff, buffer_backup);
			p_error(false);
			i--;
			continue;
		}
		res = strtok_fixed(buffer, " ", '\"');
		strcpy(metadataMemory[i].filename, res); //Set filename
		//Calculate filesize from metadata
		res = strtok_fixed(NULL, " ", '\"');
		i_buffer = atoi(res);
		if (!i_buffer) {
			error_code = FILEINIT_FAILURE_SIZE_INVALID;
			strcpy(error_buff, buffer_backup);
			p_error(false);
			i--;
			continue;
		}
		//Calculate start sector from metadata
		metadataMemory[i].size = i_buffer;
		res = strtok_fixed(NULL, " ", '\"');
		i_buffer = atoi(res);
		if (!i_buffer) {
			error_code = FILEINIT_FAILURE_SECTOR_INVALID;
			strcpy(error_buff, buffer_backup);
			p_error(false);
			i--;
			continue;
		}
		metadataMemory[i].start_sector = i_buffer;
		//Set all sectors of the current file to not-free
		sector_init(metadataMemory[i].start_sector, metadataMemory[i].size, 1);
		fgets_fixed(buffer, LINE_BUFFER_SIZE, drive, '\"', false); //Iterate on file
		removeNewLine(buffer);
		metadataMemory[i].isFree = false; //metadataMemory slot occupied
	}
}
/******************************************************
* Func: sectorSearch                                  *
* Param: none                                         *
*                                                     *
* Return: int i: first available hdd sector slot      *
******************************************************/
long sectorSearch() {
	long i;
	long numberOfFiles = (drivemetadata->totalSize / drivemetadata->sectorSize) - 1;
	for (i = 0; freeSectors[i] && i < numberOfFiles; i++)
		;
	return i >= numberOfFiles ? FILE_FAILED : i;
}
/******************************************************
* Func: metadataMemorySearch                          *
* Param: none                                         *
*                                                     *
* Return: int i: first available metadataMemory slot  *
******************************************************/
int metadataMemorySearch() {
	int i;
	long numberOfFiles = (drivemetadata->totalSize / drivemetadata->sectorSize) - 1;
	for (i = 0; metadataMemory[i].isFree == 0 && i < numberOfFiles; i++)
		;
	return i >= numberOfFiles ? FILE_FAILED : i;
}
/*******************************************************
* Func: fileSearch                                     *
* Param: const char *filename: Filename                *
*                                                      *
* Return: long sector: File's Harddrive sector address *
* (-1 if not found)                                    *
*******************************************************/
long fileSearch (const char *filename) {
	long i;
	bool found;
	long numberOfFiles = drivemetadata->totalSize / drivemetadata->sectorSize;
	for (i = 0; (found = (strcmp(metadataMemory[i].filename, filename) == 0)) != true && i < numberOfFiles; i++)
		;
	return found ? i : FILE_FAILED;
}
/***************************************************
* Func: openFile                                   *
* Param: const char *filename: Filename(Required)  *
*        FILE *drive: .bin File pointer            *
*                                                  *
* Return: none                                     *
***************************************************/
void openFile (const char *filename, FILE *drive) {
	fpos_t newpos;
	long file_pos = fileSearch(filename);
	if (file_pos == FILE_FAILED) {
		strcpy(error_buff, filename);
		error_code = FILE_NOT_FOUND;
		p_error(true);
		return;
	}
	newpos = (metadataMemory + file_pos)->start_sector * drivemetadata->sectorSize;
	fsetpos(drive, &newpos);
}
/******************************************************************************
* Func: writeToFile                                                           *
* Param: const char *text: Input string										  *
*        const char *filename: Filename(Optional), will be ignored if empty.  *
*																		      *
* Return: none																  *
* Desc: Writes a given string to a file, Will overwrite existing data.		  *
******************************************************************************/
void writeToFile(const char *text, const char *filename, FILE *drive) {
	if (!isStringEmpty(filename)) openFile (filename, drive);
	if (strlen(text) >= drivemetadata->sectorSize - 4) { // -4: "END\n"
		error_code = FILE_FULL;
		p_error(false);
		return;
	}
	fpos_t pos;
	fgetpos(drive, &pos);
	fputs(text, drive);
	fputs("\nEND\n", drive);
	fsetpos(drive, &pos); //Reset File position
}
/******************************************************************************
* Func: appendToFile                                                          *
* Param: const char *text: Input string										  *
*        const char *filename: Filename(Optional), will be ignored if empty.  *
*																		      *
* Return: none																  *
* Desc: Appends a given string to a file, no overwriting whatsoever.		  *
******************************************************************************/
void appendToFile(const char *text, char *filename, FILE *drive) {
	if (!isStringEmpty(filename)) openFile (filename, drive);
	long filesize = 0;
	fpos_t pos;
	fgetpos(drive, &pos);
	char buffer[STRING_SIZE];
	fgets_fixed(buffer, STRING_SIZE, drive, '\"', false);
	while (strcmp(buffer, "END\n") != 0 && strcmp(buffer,"END") != 0) {
		filesize += strlen(buffer);
		fgets_fixed(buffer, STRING_SIZE, drive, '\"', false); // Go to the end of the file ("END")
	}
	if (filesize + strlen(text) >= drivemetadata->sectorSize - 4) { // -4: "END\n"
		error_code = FILE_FULL;
		p_error(false);
		return;
	}
	fseek(drive, -4, SEEK_CUR); //Just before the "END\n" thing
	fputs(text, drive); //Insert the given string
	fputs("\nEND\n",drive); //And reinsert "END\n"
	fsetpos(drive, &pos); //Reset file position
}
/****************************************************
* Func: updateDriveMetadata							*
* Param: FILE *drive: .bin File pointer				*
*													*
* Return: none										*
* Desc: Updates the metadata sector of the  		*
* virtual drive & syncs it with metadataMemory		*
****************************************************/
void updateDriveMetadata(FILE *drive) {
	fpos_t pos = drivemetadata->disk_info_len;
	fsetpos(drive, &pos);
	for (int i = 0; metadataMemory[i].isFree == false; i++) {
		//There's room for optimization here ...
		fprintf(drive, "%s %d %d\n", metadataMemory[i].filename, metadataMemory[i].size, metadataMemory[i].start_sector);
	}
	fputs("END\n", drive);
	fflush(drive);
}
/***********************************************
* Func: createFile                             *
* Param: char *filename: Filename (Required)   *
*        FILE *drive: .bin File pointer        *
*                                              *
* Return: none                                 *
************************************************/
void createFile(const char *filename, FILE *drive) {
	int memorySlot = metadataMemorySearch();
	int freeSector = sectorSearch();
	fpos_t oldpos, newpos;
	fgetpos(drive, &oldpos);
	if (fileSearch(filename) != FILE_FAILED) {
		error_code = FILE_ALREADY_EXISTS;
		strcpy(error_buff, filename);
		p_error(false);
		return;
	}
	if (freeSector == FILE_FAILED) {
		error_code = DISK_FULL;
		strcpy(error_buff, filename);
		p_error(false);
		return;
	}
	strcpy(metadataMemory[memorySlot].filename, filename);
	metadataMemory[memorySlot].start_sector = freeSector;
	metadataMemory[memorySlot].size = 1;
	metadataMemory[memorySlot].isFree = 0;
	newpos = drivemetadata->sectorSize * freeSector;
	fsetpos(drive, &newpos);
	fputs("\nEND\n", drive);
	fsetpos(drive, &oldpos);
	updateDriveMetadata(drive);
	sector_init(freeSector, 1, 1);
}
/****************************************************
* Func: deleteFile                                  *
* Param: const char *filename: Filename (Required)  *
*        FILE *drive: .bin File pointer             *
*                                                   *
* Return: none                                      *
****************************************************/
void deleteFile(const char *filename, FILE *drive) {
	int memorySlot;
	if ((memorySlot = fileSearch(filename)) == FILE_FAILED) {
		error_code = FILE_DOESNT_EXISTS;
		strcpy(error_buff, filename);
		p_error(false);
		return;
	}
	metadataMemory[memorySlot].isFree = 1;
	updateDriveMetadata(drive);
	sector_init(metadataMemory[memorySlot].start_sector, metadataMemory[memorySlot].size, 0);
}
void renameFile(const char *old_name, const char *new_name, FILE *drive) {
	int memorySlot;
	if ((memorySlot = fileSearch(old_name)) == FILE_FAILED) {
		error_code = FILE_DOESNT_EXISTS;
		strcpy(error_buff, old_name);
		p_error(false);
		return;
	}
	strncpy(metadataMemory[memorySlot].filename, new_name, FILENAME_SIZE);
	updateDriveMetadata(drive);
}
/*********************************************************************************************************
* Func: readLine                                                                                         *
* Param: char *buffer: write buffer                                                                      *
*        const char *filename: Filename(optional), will be ignored if empty.                             *
*		 const bool resetPos: Reset file position to the beginning, required for the last call in a row. *
*		 FILE *drive: .bin File pointer                                                                  *
*		                                                                                                 *
* Return: none                                                                                           *
*********************************************************************************************************/
void readLine(char *buffer, const char *filename, const bool resetPos, FILE *drive) {
	fpos_t pos;
	if (filename && !isStringEmpty(filename))  //Open the given file if filename isn't empty
		openFile(filename, drive);
	fgets_fixed(buffer, LINE_BUFFER_SIZE, drive, '\"', false);
	if (resetPos) {
		fgetpos(drive, &pos);
		pos /= drivemetadata->sectorSize;
		pos *= drivemetadata->sectorSize;
		fsetpos(drive, &pos);
	} //Reset to start of the current file (same as openfile(char *filename))
}
/****************************************************************************
* Func: readChar                                                            *
* Param: constchar *filename: Filename(optional), will be ignored if empty. *
*		 const int n: nth Character                                         *
*		 FILE *drive: .bin File pointer                                     *
*                                                                           *
* Return: char n                			                                *
****************************************************************************/
unsigned char readChar (const char *filename, const int n, FILE *drive) {
	if (filename && !isStringEmpty(filename))
		 openFile (filename, drive);
	fpos_t pos, newpos;
	unsigned char buffer;
	fgetpos(drive, &pos);
	newpos = (long)pos + (long)n;
	fsetpos(drive, &newpos);
	buffer = (unsigned char)fgetc(drive);
	fsetpos(drive, &pos);
	return buffer;
}
void writeChar (const char *filename,const char c, const int n, FILE *drive) {
	if (filename && !isStringEmpty(filename))
		openFile (filename, drive);
	fpos_t pos, newpos;
	fgetpos(drive, &pos);
	newpos = (long)pos + (long)n;
	if (n > drivemetadata->sectorSize || newpos <= drivemetadata->sectorSize * 2) { //Os file and metadatasector are write protected
		error_code = WRITE_FAILED_ACCESS_DENIED;
		sprintf(error_buff, "%l", newpos);
		p_error(true);
		return;
	}
	fsetpos(drive, &newpos);
	fputc((int)c, drive);
	fsetpos(drive, &pos);
}
/***********************************************
* Func: metadataMemoryInit                     *
* Param: none                                  *
*                                              *
*                                              *
* Return: none                                 *
* Desc: Initalizes memory values to free       *
************************************************/
void metadataMemoryInit() {
	long numberOfFiles = drivemetadata->totalSize / drivemetadata->sectorSize;
	for (int i = 0; i < numberOfFiles; i++)
		metadataMemory[i].isFree = true;
}
/*********************************************************
* Func: get_disk_info                                    *
* Params: const char *filename: Drive's filename         *
*                                                        *
* Return: disk_info struct, detailes of the input drive  *
*********************************************************/
disk_info get_disk_info(const char *filename) {
	disk_info ret;
	FILE *f;
	long i_buff = 0;
	if (!(f = fopen(filename, "r"))) {
		return ret;
	}
	char buffer[STRING_SIZE];
	char *res;
	fgets(buffer, STRING_SIZE, f);
	ret.disk_info_len = strlen(buffer);
	removeNewLine(buffer);
	res = strtok(buffer, " ");
	i_buff = atoi(res);
	if (!i_buff) {
		error_code = DRIVE_SIZE_INVALID;
		p_error(false);
		ret.totalSize = DEFAULT_DRIVE_SIZE;
	} else
		ret.totalSize = i_buff;
	res = strtok(NULL, " ");
	i_buff = atoi(res);
	if (!i_buff) {
		error_code = SECTOR_SIZE_INVALID;
		p_error(false);
		ret.totalSize = DEFAULT_SECTOR_SIZE;
	} else
		ret.sectorSize = atoi(res);
	fclose(f);
	return ret;
}
void getFileInfo(long long int *dest, const int n, const int maxlen) {
	char buff[FILENAME_SIZE];
	strncpy(buff, (metadataMemory + n)->filename, FILENAME_SIZE); //Altering prevention
	if ((metadataMemory + n)->isFree)
		*dest = 0; //getFileInfo failed, no file available in that slot..
	else
		intPtoString(buff, dest, CHARP_TO_INTP, maxlen);
}

