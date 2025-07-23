#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "headers.h"
#include <string.h>
#include <dirent.h>
#include <conio.h>
//-------------------------
//Global Variables
FILE *drive, *bootdev; //Current drive and .bootdev config file (boot priority)
char *boot_drive;
//Error Handler
extern enum errors_def error_code;
//-------------------------
//Functions
/***************************************************************************
* Func: list_drives                                                        *
* Params: const bool print_flag: Print each found drive?                   *
*         char drives[][STRING_SIZE]: Where to write each found drive      *
*                                                                          *
* Return: Number of found drives                                           *
*                                                                          *
***************************************************************************/

int list_drives(const bool print_flag, char drives[][STRING_SIZE]) {
	fpos_t pos_zero = 0;
	DIR *cur_dir;
	struct dirent *filedata;
	int i = 0;
	if (!(cur_dir = opendir(DRIVES_DIR))) {
		error_code = OPEN_DIR_FAILED;
		p_error(true);
	}
	while((filedata = readdir(cur_dir)) != NULL) {
		if (strstr(filedata->d_name, DRIVE_EXT)) {
			if (print_flag) {
				printf("	• SATA Port %d : %s | Size: %s", i, filedata->d_name, humanSize(get_disk_info(filedata->d_name).totalSize));
				if (strcmp(filedata->d_name, boot_drive) == 0)
					printf(" (Boot Drive)");
				putchar('\n');
			}
			if (drives)
				strncpy(drives[i], filedata->d_name, STRING_SIZE);
			i++;
		}
	}
	if (!i && print_flag)
		printf("	No Drives Found!\n");
	return i;
}
/*****************************************************
* Func: selectBootDrive                              *
* Params: none                                       *
*                                                    *
* Return: first priority boot drive (which exists)   *
*                                                    *
*****************************************************/
char *selectBootDrive() {
	fpos_t zero = 0;
	static char boot_drive[STRING_SIZE];
	fsetpos(bootdev, &zero);
	do {
		if (!fgets(boot_drive, STRING_SIZE, bootdev)) {
			error_code = BOOTDRIVE_NOT_FOUND;
			p_error(true);
			return NULL;
		}
		removeNewLine(boot_drive);
	} while(access(boot_drive, F_OK) != 0);
	fsetpos(bootdev, &zero);
	return boot_drive;
}
/*************************************************************************
* Func: update_bootdev                                                   *
* Params: none                                                           *
*                                                                        *
* Return: none                                                           *
* Desc: Updates .bootdev, adds newly found drives to the end of bootdev  *
*************************************************************************/
void update_bootdev() {
	char drives_prim[MAX_DRIVES][STRING_SIZE], drives_bootdev[MAX_DRIVES][STRING_SIZE];
	int bootdev_cnt, prim_cnt;
	prim_cnt = list_drives(false, drives_prim);
	bootdev_cnt = fgets_lineByLine(bootdev, drives_bootdev);
	for (int i = 0; i < prim_cnt; i++) {
		for (int j = 0; j <= bootdev_cnt; j++) {
			if (j == bootdev_cnt) {
				fseek(bootdev, -1, SEEK_END);
				fputc('\n', bootdev);
				fputs(drives_prim[i], bootdev);
				fputc('\n', bootdev);
				break;
			}
			if (strcmp(drives_prim[i], drives_bootdev[j]) == 0) {
				break;
			}

		}
	}
}
/**********************************************************************************
* Func: setup_print_drives                                                        *
* Params: const int cur: Currently selected drive                                 *
*         const int cnt: Drives count                                             *
*         const char drives[][STRING_SIZE]: Drive names                           *
*                                                                                 *
* Return: none                                                                    *
* Desc: The function responsible for printing the drives in the boot menu setup   *
**********************************************************************************/
void setup_print_drives(const int cur, const int cnt, const char drives[][STRING_SIZE]) {
	for (int i = 0; i < cnt; i++) {
		if (i == cur)
			printf("• ");
		else
			printf("  ");
		printf("%s\n", drives[i]);
	}
}
/********************************************************************
* Func: boot_setup                                                  *
* Params: const char drives_bootdev: Drive names from .bootdev      *
*         const int drives_cnt: Number of drives                    *
*                                                                   *
* Return: none                                                      *
* Desc: Prints the main setup menu, and handles key inputs for it   *
********************************************************************/
void boot_setup(char drives_bootdev[][STRING_SIZE], const int drives_cnt) {
	system("cls");
	update_bootdev();
	int cur = 0;
	while (true) {
		system("cls");
		setup_print_drives(cur, drives_cnt, drives_bootdev);
		printf("\n\n ↑↓: Navigation\n");
		printf("[PGUP]: Move Upwards\n");
		printf("[PGDN]: Move Downwards\n");
		printf("[F12]: Exit\n");
		int c = _getch();
		if (c == 0 || c == 224) {
			enum keys extc = _getch();
			switch(extc) {
				case DOWNARROW:
					cur = cur == drives_cnt - 1 ? 0 : cur + 1;
					break;
				case UPARROW:
					cur = cur == 0 ? drives_cnt - 1 : cur - 1;
					break;
				case PGUP:
					shift_strings(drives_bootdev, cur, drives_cnt, BACK);
					cur = cur == 0 ? drives_cnt - 1 : cur - 1;
					break;
				case PGDN:
					shift_strings(drives_bootdev, cur, drives_cnt, FORWARD);
					cur = cur == drives_cnt - 1 ? 0 : cur + 1;
					break;
				case F12:
					system("cls");
					return;
					break;
			}
		}
	}
}
/************************************
* Func: system_boot                 *
* Params: none                      *
*                                   *
* Return: none                      *
* Desc: Initliazes stuff            *
************************************/
void systm_boot() {
	system("cls");
	drive = drive_init(boot_drive);
	memoryInit(SYSTEM_RAM);
	memoryInit(USER_RAM);
	shared_memory_handler(REG_INIT);
	return;
}
/************************************
* Func: system_shutdown             *
* Params: FILE *drive               *
*                                   *
* Return: none                      *
* Desc: Shutdowns stuff             *
************************************/
void system_shutdown(FILE *drive) {
	system("cls");
	printf("Shuting Down In 3...");
	Sleep(1000);
	printf("2...");
	Sleep(1000);
	printf("1...\n");
	printf("Flushing Random Access Memory");
	print_dots(3, 500);
	memoryInit(SYSTEM_RAM);
	memoryInit(USER_RAM);
	shared_memory_handler(REG_UNINIT);
	printf(" Done\n");
	printf("Unmounting Drive %s", boot_drive);
	print_dots(3,500);
	drive_uninit(drive);
	printf(" Done\n");
	printf("----------------------------------------\n");
	printf("All Systems Terminated. Farewell!\n");
	printf("----------------------------------------\n");
	exit(0);
}
/*******************************************************************
* Func: boot_menu                                                  *
* Params: none                                                     *
*                                                                  *
* Return: none                                                     *
* Desc: boot device selector menu (without changing any configs)   *
*******************************************************************/
void boot_menu() {
	system("cls");
	printf("Storage Devices:\n");
	char buffer[STRING_SIZE];
	char drives[MAX_DRIVES][STRING_SIZE];
	int index;
	list_drives(true, drives);
	printf("\nEnter Boot Device ID: ");
	fgets(buffer, STRING_SIZE, stdin);
	while(!(index = atoi(buffer))) {
		error_code = INVALID_INPUT_NOT_DIGITS;
		p_error(false);
		fgets(buffer, STRING_SIZE, stdin);
	}
	strcpy(boot_drive, drives[index]);
	printf("Booting from: %s", boot_drive);
	print_dots(3, 750);
	system("cls");
}
/*******************************************************************
* Func: setup_menu                                                 *
* Params: none                                                     *
*                                                                  *
* Return: none                                                     *
* Desc: Setup menu handler                                         *
*******************************************************************/
void setup_menu() {
	int drives_cnt;
	char drives_bootdev[MAX_DRIVES][STRING_SIZE];
	drives_cnt = fgets_lineByLine(bootdev, drives_bootdev);
	while (true) {
		int c;
		system("cls");
		printf("Press [INSERT] To Enter Boot Setup\n");
		printf("Press [DEL] To Exit Discarding Changes\n");
		printf("Press [F12] To Exit Saving Changes\n");
		c = _getch(); //Get key input
		if (c == 0 || c == 224) {
			enum keys extc = _getch();
			switch(extc) {
				case INSERT:
					printf("Entering Boot Setup");
					print_dots(3, 750);
					boot_setup(drives_bootdev, drives_cnt);
					break;
				case DEL:
					printf("Exiting Discarding Changes");
					print_dots(3, 750);
					putchar('\n');
					return;
					break;
				case F12:
					printf("Exiting Saving Changes");
					print_dots(3, 750);
					putchar('\n');
					fputs_lineByLine(bootdev, drives_bootdev, drives_cnt);
					boot_drive = selectBootDrive();
					return;
					break;
				default:
					break;
			}

		}
	}
}
/***********************************************************************
* Func: print_dots                                                     *
* Params: const int n: Number of dots                                  *
*         const long wait_duration: Duration between each dot print    *
* Return: none                                                         *
***********************************************************************/
void print_dots(const int n, const long wait_duration) {
	if (n > DOTS_MAX || wait_duration > DOTS_DURATION_MAX) { //Sanity check
		error_code = PRINT_DOTS_MAXIMUM_EXCEEDED;
		p_error(false);
		return;
	}
	for (int i = 0; i < n; i++) {
		putchar('.');
		Sleep(wait_duration);
	}
}
/*************************************************************************************************************
* Func: bios_post                                                                                            *
* Params: none                                                                                               *
*                                                                                                            *
* Return: none                                                                                               *
* Desc: SYSTEM POWER-ON SELF TEST, tests stuff to see if everything checks out, if yes start up the system.  *
*************************************************************************************************************/
void bios_post() {
	system("chcp 65001"); // Set terminal locale to unicode (special characters support)
	int i = 0;
	float freq = 1000 / CLOCK_PULSE;
	char *humanized_size;
	enum post_stat status;
	clock_t start, elapsed = 0;
	if (!(bootdev = fopen(BOOTDEV_CFG_FILENAME, "rb+"))) {
		error_code = OPENING_BOOTDEV_FAILED;
		p_error(true);
	}
	while (true) {
		system("cls");
		printf("========================================\n");
		printf("         SYSTEM POWER-ON SELF TEST      \n");
		printf("         BIOS v1.00PG - Build 0001      \n");
		printf("          © QuickhandRobert - 2025      \n");
		printf("========================================\n\n");
		printf("▸ CPU       : Running on %0.4fHz, OK\n", freq);
		humanized_size = humanSize((MEMORY_SIZE * (sizeof(userRAM) + sizeof(systemRAM))));
		printf("▸ RAM       : %s, OK (Dual Channel)\n\n", humanized_size);
		printf("Storage Devices:\n");
		boot_drive = selectBootDrive(); //Determine boot drive, will exit if not found..
		list_drives(true, NULL); //Print currently available drives
		update_bootdev(); //Update .bootdev if new drives are found
		putchar('\n');
		printf("▪ Keyboard  : USB HID Keyboard (OK)\n");
		printf("▪ Mouse     : USB Optical Mouse (OK)\n\n"); //ummm.. yeah.. this is bullshit :P
		printf("✅ POST Status: All Systems Operational\n\n");
		printf("----------------------------------------\n");
		printf("Press [DEL] to Enter Setup\n");
		printf("Press [INSERT] to Access Boot Menu\n");
		printf("Booting from: %s", boot_drive);
		start = clock();
		elapsed = 0;
		status = BOOT_OK;
		while (elapsed < POST_WAIT_DURATION) {
			if (_kbhit()) {
				int c = _getch();
				if (c == 0 || c == 224) {
					enum keys extc = _getch();
					if (extc == DEL) {
						status = SETUP;
						break;
					} else if (extc == INSERT) {
						status = BOOT_MENU;
						break;
					}
				}
			}
			elapsed = clock() - start;
			if (elapsed < (POST_WAIT_DURATION / 3) && i == 0) {
				putchar('.');
				i++;
			} else if (elapsed > POST_WAIT_DURATION / 3 && i == 1) {
				putchar('.');
				i++;
			} else if (i == 2 && elapsed > (POST_WAIT_DURATION * 0.66f)) {
				putchar('.');
				i++;
			}
		}
		printf("\n----------------------------------------\n");
		switch(status) {
			case BOOT_MENU:
				printf("Entering Boot Menu");
				print_dots(3, 750);
				boot_menu();
				return;
				break;
			case SETUP:
				printf("Entering Setup Menu");
				print_dots(3, 750);
				setup_menu();
				break;
			case BOOT_OK:
				return;
				break;
		}
	}
}
int main() {
	bios_post();
	systm_boot();
	loadToMemory(drive, "");
	runCPU(drive);
}
