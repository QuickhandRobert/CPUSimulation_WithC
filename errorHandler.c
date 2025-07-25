#include <stdio.h>
#include <stdlib.h>
#include "headers.h"
enum errors_def error_code;
char error_buff[STRING_SIZE];
void p_error(bool terminate) {
	fprintf(stderr, "\n[KERNEL] ");
	switch (error_code) {
		case OPEN_DIR_FAILED:
			fprintf(stderr, "Couldn\' open directory %s. ", DRIVES_DIR);
			perror("");
			break;
		case BOOTDRIVE_NOT_FOUND:
			fprintf(stderr, "No bootable media found.\n");
			break;
		case INVALID_INPUT_NOT_DIGITS:
			fprintf(stderr, "Input value is not a number.\n");
			break;
		case PRINT_DOTS_MAXIMUM_EXCEEDED:
			fprintf(stderr, "Printing Dots failed, Maximum exceeded.\n");
			break;
		case OPENING_BOOTDEV_FAILED:
			fprintf(stderr, "Opening .bootdev config file failed. ");
			perror("");
			break;
		case FILE_EMPTY:
			fprintf(stderr, "Drive initialization failed, drive is empty.\n");
			break;
		case FILEINIT_FAILURE_FILENAME_NOTPRESENT:
			fprintf(stderr, "Drive initialization failed, %s filename is invalid.\n", error_buff);
			break;
		case FILEINIT_FAILURE_SIZE_INVALID:
			fprintf(stderr, "Drive initialization failed, %s filesize is invalid.\n", error_buff);
			break;
		case FILEINIT_FAILURE_SECTOR_INVALID:
			fprintf(stderr, "Drive initialization failed, %s sector number is invalid.\n", error_buff);
			break;
		case FILE_NOT_FOUND:
			fprintf(stderr, "Error loading file %s, file not found", error_buff);
			break;
		case FILE_FULL:
			fprintf(stderr, "File is full, Maximum sector size exceeded.\n");
			break;
		case DISK_FULL:
			fprintf(stderr, "Creating file %s failed. Disk is full.\n", error_buff);
			break;
		case FILE_ALREADY_EXISTS:
			fprintf(stderr, "Creating file %s failed. File already exists.\n", error_buff);
			break;
		case FILE_DOESNT_EXISTS:
			fprintf(stderr, "Deleting file %s failed. File doesn\'t exist.\n", error_buff);
			break;
		case DRIVE_SIZE_INVALID:
			fprintf(stderr, "Couldn\'t read drive metadata, Drive size invalid. Using default value: %d\n", DEFAULT_DRIVE_SIZE);
			break;
		case SECTOR_SIZE_INVALID:
			fprintf(stderr, "Couldn\'t read drive metadata, Sector size invalid. Using default value: %d\n", DEFAULT_SECTOR_SIZE);
			break;
		case MEMORY_LIMIT_EXCEEDED:
			fprintf(stderr, "Memory Limit Exceeded. Memory Address %s does not exist.\n", error_buff);
			break;
		case CPU_LIMIT_EXCEEDED:
			fprintf(stderr, "CPU limit exceeded.\n");
			break;
		case ERROR_DEFINITION_FAILED_INVALID_INDEX:
			fprintf(stderr, "Syntax error: Error definition failed, %s is not a number.\n", error_buff);
			break;
		case CONST_DEFINITION_FAILED_INVALID_INDEX:
			fprintf(stderr, "Syntax error: Constant string definition failed, %s is not a number.\n", error_buff);
			break;
		case ERROR_CALL_FAILED_INVALID_INDEX:
			fprintf(stderr, "Syntax error: Error print failed, %s is not a number.\n", error_buff);
			break;



	}
	if (terminate) {
		printf("-----------------------------------------------------\n");
		printf("[KERNEL] System halt requested, shutting down systems");
		print_dots(3, 750);
		exit(1);
	}
}