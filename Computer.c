#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "headers.h"
#include <string.h>
#include <dirent.h>
#include <conio.h>
#include <process.h>
//-------------------------
//Global Variables
FILE *drive, *bootdev, *timezone_cfg; //Current drive and .bootdev bootdev file (boot priority)
char *boot_drive;
//Thread Creation (system poweroff trigger)
HANDLE thread_handle;
long thread_id;
//Error Handler
extern enum errors_def error_code;
//Timezone handling
int current_timezone;
time_zone_t *timezones;
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
static int list_drives(const bool print_flag, char drives[][STRING_SIZE]) {
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
static char *selectBootDrive() {
	static char boot_drive[STRING_SIZE];
	char buff[STRING_SIZE];
	rewind(bootdev);
	do {
		if (!fgets(boot_drive, STRING_SIZE, bootdev)) {
			error_code = BOOTDRIVE_NOT_FOUND;
			p_error(true);
			return NULL;
		}
		removeNewLine(boot_drive);
		sprintf(buff, "%s/%s", DRIVES_DIR, boot_drive);
	} while(access(buff, F_OK) != 0);
	rewind(bootdev);
	return boot_drive;
}
/*************************************************************************
* Func: update_bootdev                                                   *
* Params: none                                                           *
*                                                                        *
* Return: none                                                           *
* Desc: Updates .bootdev, adds newly found drives to the end of bootdev  *
*************************************************************************/
static void update_bootdev() {
	char drives_prim[MAX_DRIVES][STRING_SIZE], drives_bootdev[MAX_DRIVES][STRING_SIZE];
	int bootdev_cnt, prim_cnt;
	//Update current timezone into .bootdev
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
static void setup_print_drives(const int cur, const int cnt, const char drives[][STRING_SIZE]) {
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
static void boot_setup(char drives_bootdev[][STRING_SIZE], const int drives_cnt) {
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
/*****************************************************
* Func: get_currentTimezone                          *
* Params: none                                       *
*                                                    *
* Return: Timezone index                             *
* Desc: reads the config data from .timezone file    *
*       and returns it                               *
*****************************************************/
int get_currentTimezone() {
	char buff[STRING_SIZE];
	rewind(timezone_cfg);
	fgets(buff, STRING_SIZE, timezone_cfg);
	removeNewLine(buff);
	rewind(timezone_cfg);
	return atoi(buff);
}
/*****************************************************
* Func: get_currentTimeZone_offset                   *
* Params: none                                       *
*                                                    *
* Return: Timezone offset from GMT                   *
*****************************************************/
time_t get_currentTimeZone_offset () {
	return (timezones + current_timezone)->difference;
}
/********************************************
* Func: timezones_init                      *
* Params: none                              *
*                                           *
* Return: none                              *
* Desc: timezones array initilization       *
********************************************/
static void timezones_init() {
	static time_zone_t init_timezones[] = {
		{-43200, "UTC-12:00 (Baker Island, Howland Island)"},
		{-39600, "UTC-11:00 (Niue, American Samoa)"},
		{-36000, "UTC-10:00 (Hawaii Standard Time, Tahiti)"},
		{-34200, "UTC-09:30 (Marquesas Islands)"},
		{-32400, "UTC-09:00 (Alaska Standard Time, Gambier)"},
		{-28800, "UTC-08:00 (Pacific Standard Time, Tijuana)"},
		{-25200, "UTC-07:00 (Mountain Standard Time, Denver)"},
		{-21600, "UTC-06:00 (Central Standard Time, Chicago)"},
		{-18000, "UTC-05:00 (Eastern Standard Time, New York)"},
		{-16200, "UTC-04:30 (Caracas)"},
		{-14400, "UTC-04:00 (Atlantic Standard Time, Halifax)"},
		{-12600, "UTC-03:30 (Newfoundland Standard Time)"},
		{-10800, "UTC-03:00 (Argentina, Brazil, Greenland)"},
		{-7200, "UTC-02:00 (South Georgia)"},
		{-3600, "UTC-01:00 (Azores, Cape Verde)"},
		{0, "UTC+00:00 (Coordinated Universal Time, London)"},
		{3600, "UTC+01:00 (Central European Time, Paris)"},
		{7200, "UTC+02:00 (Eastern European Time, Cairo)"},
		{10800, "UTC+03:00 (Moscow, Riyadh)"},
		{12600, "UTC+03:30 (Tehran)"},
		{14400, "UTC+04:00 (Dubai, Baku)"},
		{16200, "UTC+04:30 (Kabul)"},
		{18000, "UTC+05:00 (Pakistan, Maldives)"},
		{19800, "UTC+05:30 (India Standard Time, Mumbai)"},
		{20700, "UTC+05:45 (Nepal)"},
		{21600, "UTC+06:00 (Bangladesh, Bhutan)"},
		{23400, "UTC+06:30 (Cocos Islands, Myanmar)"},
		{25200, "UTC+07:00 (Thailand, Vietnam)"},
		{28800, "UTC+08:00 (China Standard Time, Perth)"},
		{30600, "UTC+08:30 (North Korea)"},
		{32400, "UTC+09:00 (Japan Standard Time, Seoul)"},
		{34200, "UTC+09:30 (Australian Central Standard Time)"},
		{36000, "UTC+10:00 (Australian Eastern Standard Time)"},
		{37800, "UTC+10:30 (Lord Howe Island)"},
		{39600, "UTC+11:00 (Solomon Islands, Vanuatu)"},
		{43200, "UTC+12:00 (Fiji, New Zealand)"},
		{46800, "UTC+13:00 (Tonga, Phoenix Islands)"},
		{50400, "UTC+14:00 (Line Islands, Kiribati)"}
	};
	timezones = init_timezones;
	current_timezone = get_currentTimezone();
}
/********************************************
* Func: update_timezone                     *
* Params: none                              *
*                                           *
* Return: none                              *
* Desc: Updates the config file             *
********************************************/
static void update_timezone() {
	rewind(timezone_cfg);
	fprintf(timezone_cfg, "%d\n", current_timezone);
	rewind(timezone_cfg);
}
/********************************************
* Func: setup_print_timezones               *
* Params: none                              *
*                                           *
* Return: none                              *
* Desc: Timezones setup helper function     *
********************************************/
static void setup_print_timezones() {
	char print_buffer[STRING_SIZE * NUMBER_OF_TIMEZONES];
	size_t current_offset = 0;
	for (int i = 0; i < NUMBER_OF_TIMEZONES; i++)
		current_offset += snprintf(print_buffer + current_offset, STRING_SIZE, i == current_timezone ? "[ • ] %s\n" : "[   ] %s\n", (timezones + i) -> desc);
	fputs(print_buffer, stdout);
}
/********************************************
* Func: timezone_setup                      *
* Params: none                              *
*                                           *
* Return: none                              *
********************************************/
static void timezone_setup() {
	system("cls");
	update_timezone();
	while (true) {
		system("cls");
		setup_print_timezones();
		printf("\n\n ↑↓: Navigation\n");
		printf("[F12]: Exit\n");
		int c = _getch();
		if (c == 0 || c == 224) {
			enum keys extc = _getch();
			switch(extc) {
				case DOWNARROW:
					current_timezone = current_timezone == NUMBER_OF_TIMEZONES - 1 ? 0 : current_timezone + 1;
					break;
				case UPARROW:
					current_timezone = current_timezone == 0 ? NUMBER_OF_TIMEZONES - 1 : current_timezone - 1;
					break;
				case F12:
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
static void systm_boot() {
	char buff[STRING_SIZE];
	system("cls");
	sprintf(buff, "%s/%s", DRIVES_DIR, boot_drive);
	drive = drive_init(buff);
	audio_init();
	memoryInit(SYSTEM_RAM);
	memoryInit(USER_RAM);
	thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)watch_for_poweroff, NULL, 0, &thread_id);
}
/************************************
* Func: system_shutdown             *
* Params: FILE *drive               *
*                                   *
* Return: none                      *
* Desc: Shutdowns stuff             *
************************************/
void system_shutdown() {
	remove(PAGEFILE_FILENAME);
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
	printf(" Done\n");
	printf("Unmounting Drive %s", boot_drive);
	print_dots(3,500);
	drive_uninit(drive);
	printf(" Done\n");
	printf("----------------------------------------\n");
	printf("All Systems Terminated. Farewell!\n");
	printf("----------------------------------------\n");
	shared_memory_handler(REG_UNINIT, false);
	audio_shutdown();
	ExitProcess(0);
}
void system_hibernate() {
	remove(PAGEFILE_FILENAME);
	pgfile_write();
	system("cls");
	printf("Hibernating In 3...");
	Sleep(1000);
	printf("2...");
	Sleep(1000);
	printf("1...\n");
	printf("Paging Registers & Random Access Memory");
	print_dots(3, 500);
	printf(" Done\n");
	printf("Unmounting Drive %s", boot_drive);
	print_dots(3,500);
	drive_uninit(drive);
	printf(" Done\n");
	printf("----------------------------------------\n");
	printf("All Systems Terminated. Farewell!\n");
	printf("----------------------------------------\n");
	shared_memory_handler(REG_UNINIT, false);
	audio_shutdown();
	exit(0);
}

/************************************
* Func: system_restart              *
* Params: FILE *drive               *
*                                   *
* Return: none                      *
* Desc: Restarts stuff              *
************************************/
void system_restart() {
	char exe_path[STRING_SIZE];
	char run_buff[STRING_SIZE];
	STARTUPINFO s_info;
	PROCESS_INFORMATION p_info;
	//Initialize to default values, because, we're dealing with the Windows API here, and nothing makes sense there
	memset(&s_info, 0, sizeof(s_info));
	memset(&p_info, 0, sizeof(p_info));
	s_info.cb = sizeof(s_info);
	GetModuleFileName(NULL, exe_path, STRING_SIZE);
	sprintf(run_buff, "\"%s\" %s", exe_path, SYSTEM_RESTART_TRIGGER);
	system("cls");
	printf("Restarting In 3...");
	Sleep(1000);
	printf("2...");
	Sleep(1000);
	printf("1...\n");
	printf("Flushing Random Access Memory");
	print_dots(3, 500);
	memoryInit(SYSTEM_RAM);
	memoryInit(USER_RAM);
	printf(" Done\n");
	printf("Unmounting Drive %s", boot_drive);
	print_dots(3,500);
	drive_uninit(drive);
	printf(" Done\n");
	system("cls");
	shared_memory_handler(REG_UNINIT, false);
	audio_shutdown();
	CreateProcess(NULL, run_buff, NULL, NULL, false, 0, NULL, NULL, &s_info, &p_info);
	CloseHandle(p_info.hProcess);
	CloseHandle(p_info.hThread);
	ExitProcess(0);
}
/*******************************************************************
* Func: boot_menu                                                  *
* Params: none                                                     *
*                                                                  *
* Return: none                                                     *
* Desc: boot device selector menu (without changing any bootdevs)   *
*******************************************************************/
static void boot_menu() {
	system("cls");
	printf("Storage Devices:\n");
	char buffer[STRING_SIZE];
	char drives[MAX_DRIVES][STRING_SIZE];
	int index;
	list_drives(true, drives);
	printf("\nEnter Boot Device ID: ");
	fgets(buffer, STRING_SIZE, stdin);
	while(!isdigit((int)*buffer)) {
		error_code = INVALID_INPUT_NOT_DIGITS;
		p_error(false);
		printf("\nEnter Boot Device ID: ");
		fgets(buffer, STRING_SIZE, stdin);
	}
	index = atoi(buffer);
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
static void setup_menu() {
	int drives_cnt;
	char drives_bootdev[MAX_DRIVES][STRING_SIZE];
	drives_cnt = fgets_lineByLine(bootdev, drives_bootdev);
	while (true) {
		int c;
		system("cls");
		printf("Press [INSERT] To Enter Boot Setup\n");
		printf("Press [F11] To Enter Time Zone Settings\n");
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
				case F11:
					printf("Entering Timezone Setup");
					print_dots(3, 750);
					timezone_setup();
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
					update_timezone();
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
static void bios_post() {
	system("chcp 65001"); // Set terminal locale to unicode (special characters support)
	int i;
	float freq = 1000 / CLOCK_PULSE;
	char *humanized_size;
	enum post_stat status;
	clock_t start, elapsed = 0;
	if (!(bootdev = fopen(BOOTDEV_CFG_FILENAME, "rb+"))) {
		error_code = OPENING_BOOTDEV_FAILED;
		p_error(true);
	}
	if (!(timezone_cfg = fopen(TIMEZONE_CFG_FILENAME, "rb+"))) {
		error_code = OPENING_TIMEZONECFG_FAILED;
		p_error(true);
	}
	//Timezone stuff
	timezones_init();
	while (true) {
		system("cls");
		printf("========================================\n");
		printf("         SYSTEM POWER-ON SELF TEST      \n");
		printf("         BIOS v1.00PG - Build 0001      \n");
		printf("          © QuickhandRobert - 2025      \n");
		printf("========================================\n\n");
		printf("▸ CPU       : Running on %0.4fHz, OK\n", freq);
		humanized_size = humanSize((MEMORY_SIZE * (sizeof(userRAM_t) + sizeof(systemRAM_t))));
		printf("▸ RAM       : %s, OK (Dual Channel)\n\n", humanized_size);
		printf("Storage Devices:\n");
		boot_drive = selectBootDrive(); //Determine boot drive, will exit if not found..
		list_drives(true, NULL); //Print currently available drives
		update_bootdev(); //Update .bootdev if new drives are found
		putchar('\n');
		printf("▪ Keyboard  : USB HID Keyboard (OK)\n");
		printf("▪ Mouse     : USB Optical Mouse (OK)\n\n"); //ummm.. yeah.. this is bullshit :P
		printf("\n🕒 Current Time Zone: %s\n", (timezones + current_timezone)->desc);
		printf("✅ POST Status: All Systems Operational\n\n");
		printf("----------------------------------------\n");
		printf("Press [DEL] to Enter Setup\n");
		printf("Press [INSERT] to Access Boot Menu\n");
		printf("Booting from: %s", boot_drive);
		start = clock();
		elapsed = 0;
		i = 0;
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
/*************************************
* Func: monitor_init                 *
* Params: bool restart flag          *
*                                    *
* Return: none                       *
* Desc: starts the monitor process   *
*************************************/
static void monitor_init(bool restart_flag) {
	if (!restart_flag) {
		STARTUPINFO s_info;
		PROCESS_INFORMATION p_info;
		//Initialize to default values, because, we're dealing with the Windows API here, therefore nothing should make sense
		memset(&s_info, 0, sizeof(s_info));
		memset(&p_info, 0, sizeof(p_info));
		s_info.cb = sizeof(s_info);
		CreateProcess("monitor.exe", NULL, NULL, NULL, true, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &s_info, &p_info);
		CloseHandle(p_info.hProcess);
		CloseHandle(p_info.hThread);
	}
}
/***********************************
* Func: pgfile_write_drive_pos     *
* Params: FILE *fp                 *
*                                  *
* Return: none                     *
***********************************/
void pgfile_write_drive_pos(FILE *fp) {
	fpos_t drive_pos;
	fgetpos(drive, &drive_pos);
	fwrite(&drive_pos, sizeof(fpos_t), 1, fp);
	return;
}
/***********************************
* Func: pgfile_load_drive_pos      *
* Params: FILE *fp                 *
*                                  *
* Return: none                     *
***********************************/
void pgfile_load_drive_pos(FILE *fp) {
	fpos_t drive_pos;
	fread(&drive_pos, sizeof(size_t), 1, fp);
	fsetpos(drive, &drive_pos);
}
int main(int argc, char *argv[]) {
	bool restart_flag = (argc > 1) && (strcmp(argv[1], SYSTEM_RESTART_TRIGGER) == 0 || strcmp(argv[1], SYSTEM_NO_MONITOR) == 0);
	bool hibernate_flag = (access(PAGEFILE_FILENAME, 0) != -1);
	shared_memory_handler(REG_INIT, restart_flag);
	monitor_init(restart_flag);
	wait_for_power(restart_flag);
	bios_post();
	systm_boot();
	if (hibernate_flag)
		pgfile_load();
	else
		loadToMemory(drive, "");
	runCPU(drive, hibernate_flag);

}
