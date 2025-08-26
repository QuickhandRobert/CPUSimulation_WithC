#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>
//---------------------------------
//Nuklear, GLFW & GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <nuklear.h>
#include <nuklear_glfw_gl3.h>
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
//---------------------------------
//General Definitions
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 700
#define FONT_PATH "Fonts/Roboto.ttf"
#define POWER_OFF_ICON_PATH "power_off.png"
#define POWER_ON_ICON_PATH "power_on.png"
#define RESTART_ICON_PATH "restart.png"
#define CIRCLES_ACTIVE_PATH "circles/circle_active_%d.png"
#define CIRCLES_PATH "circles/circle_%d.png"
#define SYNTAX_LIMIT 8
#define STRING_SIZE 256
#define REG_STRING_SIZE 2 * STRING_SIZE
#define CONSTANT_STRINGS 128
#define WATCH_MENU_ROWS_DEFAULT 8
#define WATCH_MENU_ROWS_MAX 16
#define MEMORY_SIZE 8192
#define REG_CHARACTER_LIMIT 64
#define PROCESS_RESTART_INTERVAL 1000 //In ms
#define FETCH 0
#define DECODE 1
#define EXECUTE 2
#define nk_pi 3.14159265359f
#define HASH_KEY 5381
#define INDEX_SEPERATOR ":"
#define VIEW_DECIMAL 0
#define VIEW_HEX 1
#define VIEW_CHAR 2
#define REG_ADD 0
#define REG_REMOVE 1
#define REG_DISABLED 0
#define REG_SUCCESS 1
#define REG_ERROR 2
#define REGISTER_LABELS_UPDATE_RATE 0.1 //In seconds
#define NK_FLAGS_ALERT_WINDOW NK_WINDOW_MOVABLE | NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
//---------------------------------
//Structs
struct R_IR_INSTRUCTION {
	char inst_name[CONSTANT_STRINGS];
	int inst_params;
};
typedef struct CPU_registers {
	char R_IR[SYNTAX_LIMIT][STRING_SIZE];
	long long int R_PC;
	long long int R_SP; //Stack Pointer
	long long int R_AC; //Accumulator
	long long int R_MAR; //Memory Address
	long long int R_MDR; //Value
	long long int R_U; //General Purpose
	long long int R_V; //General Purpose
	long long int R_X; //General Purpose (Unsigned)
	long long int R_Y; //General Purpose
	long long int R_Z; //General Purpose
	long long int R_C; //Clock accumalator
	struct R_IR_INSTRUCTION R_IR_INST;
	long long R_S[REG_STRING_SIZE]; //String Handling
	long long R_A[REG_STRING_SIZE]; //String Handling
	char CONSTSTR[CONSTANT_STRINGS][STRING_SIZE];
	int step;
	bool cp_toggle;
	bool cp_differ_toggle;
	int CPU_Clock;
	volatile bool power_state;
	volatile bool restart_trigger;
	long p_id;
	volatile bool p_id_trigger;
} CPU_registers_t;
typedef struct CPU_registers_cache {
	int R_PC;
	int R_AC;
} CPU_registers_cache_t;
typedef struct registerP {
	long long int *p;
	int viewType;
	char name[STRING_SIZE];
} registerP_t;
enum registers_hashed {
	SP  = 5862728,
	AC  = 5862121,
	MAR = 193463077,
	MDR = 193463176,
	U   = 177658,
	V   = 177659,
	X   = 177661,
	Y   = 177662,
	Z   = 177663,
	S   = 177656,
	A   = 177638,
	PC  = 5862616,
	C   = 177640
};
typedef double glfw_time;
//---------------------------------
//Global Variables
int reg_stack_top = 0;
HANDLE hMapFile;
HANDLE hParentProcess;
CPU_registers_t *CPU_Registers;
CPU_registers_cache_t CPU_Registers_Cache;
registerP_t registers[WATCH_MENU_ROWS_MAX];
//---------------------------------
//Functions
/**************************************************
* Func: reg_init                                  *
* Param: none                                     *
*                                                 *
* Return: none                                    *
* Initializes Shared Memory (fileMap)             *
**************************************************/
void reg_init() {
	if (hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "Global\\SharedMemory"))
		CPU_Registers = (CPU_registers_t *) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CPU_registers_t));
	else {
		fprintf(stderr, "[FILEMAP] Couldn\'t Initialize Shared Memory\n");
		exit(1);
	}
}
bool is_parent_alive(HANDLE hParent) {
	if (!hParent)
		return false;
	long result = WaitForSingleObject(hParent, 0);
	return result == WAIT_TIMEOUT;
}
/***************************************************************************************************
* Func: indicators_eval                                                                            *
* Param: struct nk_style_button *state_indicators: Output Array of Button Styles                   *
*        const struct nk_style_button deactive: Deactive Button Style                              *
*        const struct nk_style_button active: Active Button Style                                  *
*                                                                                                  *
* Return: none                                                                                     *
* Evaluates the currently active state of the machine and assigns the correlating style to it.     *
*                                                                                                  *
***************************************************************************************************/
void indicators_eval(struct nk_style_button *state_indicators, const struct nk_style_button deactive, const struct nk_style_button active) {
	for (int i = 0; i < 3; i++) {
		state_indicators[i] = deactive;
	}
	state_indicators[CPU_Registers->step] = active; //Set the currently active status indicator's style to active (background color)
}
/********************************************
* Func: hashStr                             *
* Params: char *str: Input String           *
*                                           *
* Return: Hashed Value                      *
********************************************/
unsigned long hashStr(char *str) {
	unsigned long hash = HASH_KEY;
	char *c;
	for (c = str; *c != '\0'; c++)
		hash = ((hash << 5) + hash) + *c;
	return hash;
}
/*********************************************************************************
* Func: makeRegisterPointer                                                      *
* Params: const char *reg_id: Input Register's Name                              *
*                                                                                *
* Return: int pointer to the input register, from the shared memory (fileMap)    *
*********************************************************************************/
long long int *makeRegisterPointer(const char *reg_id) {
	char *res;
	char reg_id_cpy[STRING_SIZE];
	int index = 0;
	strncpy(reg_id_cpy, reg_id, STRING_SIZE);
	if (res = strstr(reg_id_cpy, INDEX_SEPERATOR)) { //Check if the input register has an index
		*res = '\0'; //If yes tokenize it into two parts, and extract the index from it...
		res++;
		index = atoi(res);
		if (index >= MEMORY_SIZE)
			return NULL;
	}
	enum registers_hashed hashed = hashStr(reg_id_cpy);
	switch(hashed) {
		case SP:
			return &CPU_Registers->R_SP;
		case AC:
			return &CPU_Registers->R_AC;
		case MAR:
			return &CPU_Registers->R_MAR;
		case MDR:
			return &CPU_Registers->R_MDR;
		case U:
			return &CPU_Registers->R_U;
		case V:
			return &CPU_Registers->R_V;
		case X:
			return &CPU_Registers->R_X;
		case Y:
			return &CPU_Registers->R_Y;
		case Z:
			return &CPU_Registers->R_Z;
		case S:
			return CPU_Registers->R_S + index;
		case A:
			return CPU_Registers->R_A + index;
		case PC:
			return &CPU_Registers->R_PC;
		case C:
			return &CPU_Registers->R_C;
		default:
			return NULL;

	}
}
/*************************************************************
* Func: mem_shiftback                                        *
* Params: void *data: Input Data                             *
*         const size_t element_size                          *
*         int index: Start Index                             *
*         const int count: Data Count                        *
* Return: none                                               *
*************************************************************/
void mem_shiftback(void *data, const size_t element_size, int index, const int count) {
	for (; index < count; index++) {
		memmove(data + element_size * (index), data + element_size * (index + 1), element_size);
	}
}
/********************************************************************
* Func: regWatch_init                                               *
* Params: const int viewType: VIEW_DECIMAL, VIEW_HEX, VIEW_CHAR     *
*         const char *reg_id: Register name                         *
*                                                                   *
* Return: True if successful                                        *
********************************************************************/
bool regWatch_init(const int viewType, const char *reg_id) {
	long long int *reg;
	if (reg = makeRegisterPointer(reg_id)) {
		registers[reg_stack_top].p = reg;
		registers[reg_stack_top].viewType = viewType;
		strcpy(registers[reg_stack_top].name, reg_id);
		reg_stack_top++;
		return true;
	} else {
		return false;
	}
}
/*****************************************
* Func: regWatch_uninit                  *
* Params: int index                      *
*                                        *
* Return: none                           *
*****************************************/
void regWatch_uninit(int index) {
	reg_stack_top--;
	if (index == reg_stack_top) //Do nothing
		return;
	else
		mem_shiftback(registers, sizeof(registerP_t), index, reg_stack_top); //Shift the stack backwards
}
/**************************************************************
* Func: texture_init                                          *
* Params: unsigned char *data: Image data from stb            *
*         int width                                           *
*         int height                                          *
*                                                             *
* Return: GLuint (unsigned int) to image ID on the GPU side   *
**************************************************************/
GLuint texture_init(stbi_uc *data, int width, int height) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

// Upload image data to GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind
// Free image data after uploading to GPU
	stbi_image_free(data);
	return texture;
}
/****************************************************
* Func: img_load                                    *
* Params: const char *img_name: Path to the image   *
*                                                   *
* Return: struct nk_image: initialized nk_image     *
*                                                   *
****************************************************/
struct nk_image img_load(const char *img_name) {
	int width, height, chs;
	stbi_uc *data = stbi_load(img_name, &width, &height, &chs, 4); //Load image data
	GLuint texture;
	if (data)
		texture = texture_init(data, width, height); //Upload texture data to GPU
	else {
		fprintf(stderr, "[STB_IMAGE] Couldn\'t open image %s, %s.\n", img_name, stbi_failure_reason());
		exit(1);
	}
	return nk_image_id(texture); //Assign nk_image struct to the uploaded texture
}
/****************************************************
* Func: gui_shutdown                                *
* Params: struct nk_glfw *glfw                      *
*                                                   *
* Return: none                                      *
*                                                   *
****************************************************/
void gui_shutdown(struct nk_glfw *glfw, GLFWwindow *main_win, HANDLE hParent) {
	nk_glfw3_shutdown(glfw);
	glfwDestroyWindow(main_win);
	glfwTerminate();
	UnmapViewOfFile(CPU_Registers);
	CloseHandle(hMapFile);
	CloseHandle(hParent);
}
char *humanize_freq(char *buff, int freq) {
	char *suffix[] = {"Hz", "Khz", "Mhz"};
	char length = sizeof(suffix) / sizeof(suffix[0]);
	int i = 0;
	double dblFreq = freq;
	if (freq > 1000) {
		for (i = 0; (freq / 1000) > 0 && i < length - 1; i++, freq /= 1000) {
			dblFreq = freq / 1000;
		}
	}
	sprintf(buff, "%.1f %s", dblFreq, suffix[i]);
	return buff;
}
int main() {
	reg_init();	//Initialize register's sharedmemory (with pc.exe)
	bool powerState = false;
	glfw_time current = 0, previous = 0;
	long p_reset_timer = 0, p_reset_timer_start = 0;;
	//---------------------------------
	//Clock Frequency and Duration Stuff
	float clock_freq = 1000 / (float)(CPU_Registers->CPU_Clock);
	const float clock_freq_unit = clock_freq;
	//---------------------------------
	//Register Strings
	char S_PC[REG_CHARACTER_LIMIT] = {'\0'};
	char S_IR[REG_CHARACTER_LIMIT] = {'\0'};
	char S_AC[REG_CHARACTER_LIMIT] = {'\0'};
	char S_CPUClock[REG_CHARACTER_LIMIT] = {'\0'};
	humanize_freq(S_CPUClock, clock_freq_unit);
	sprintf(S_PC, "%04d", CPU_Registers->R_PC); //Program Counter
	sprintf(S_AC, "%04d", CPU_Registers->R_AC); //Accumalator
	//---------------------------------
	//Character Buffers
	char reg_id[STRING_SIZE] = "Register\'s Name";
	char temp_buffer[STRING_SIZE];
	char *watchMenuToggleBuffer[] = {"Open Debug Menu", "Close Debug Menu"};
	//---------------------------------
	//State indicators Buttons
	struct nk_style_button state_indicators[3]; // 0:Fetch, 1:Decode, 2:Execute
	int knobState = 0;
	//---------------------------------
	//Additional window triggers
	bool watchMenu = false, addMenu = false, typeMenu = false, copiedAlert = false;
	int successErrorWindow = REG_DISABLED;
	//---------------------------------
	//Glfw window
	struct nk_glfw glfw = {0};
	static GLFWwindow *main_win; //Glfw window
	struct nk_context *ctx; //Nuklear context
	//---------------------------------
	//Images
	struct nk_image power[2], restart, circle[10], circle_active[10];
	//---------------------------------
	//Styles
	struct nk_style_button status_active;
	struct nk_style_button status_deactive;
	struct nk_style_button border_only;
	struct nk_style_button no_border;
	struct nk_style_button cp;
	struct nk_rect knob_image_bounds = nk_rect(158.8f, 234, 200, 200); //200pixels by 200 pixels, exactly where the knob is rendered
	struct nk_rect knob_text_bounds = nk_rect(223.8f, 319, 200, 200); //Bounds for textbox in the center of the knob
	//---------------------------------
	//Colors
	struct nk_color c_darkyellow = nk_rgb(227, 235, 16);
	struct nk_color c_gray = nk_rgb(50, 50, 50);
	struct nk_color c_lightgray = nk_rgb(100, 100, 100);
	//---------------------------------
	//Glfw window init
	if (!glfwInit()) {
		gui_shutdown(&glfw, main_win, hParentProcess);
		fprintf(stderr, "[GFLW] failed to init!\n");
		exit(1);
	}
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	main_win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "System Monitor", NULL, NULL); //Create glfw window
	glfwMakeContextCurrent(main_win);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		gui_shutdown(&glfw, main_win, hParentProcess);
		fprintf(stderr, "[GLEW] failed to init!\n");
		exit(1);
	}
	ctx = nk_glfw3_init(&glfw, main_win, NK_GLFW3_INSTALL_CALLBACKS); //Nuklear contex init using the pre-initialized glfw window
	ctx->style.window.fixed_background = nk_style_item_color(nk_black); // Change the background color to BLACK
	//---------------------------------
	//Fonts
	struct nk_font_atlas *atlas;
	struct nk_font *roboto;
	struct nk_font *roboto_smaller;
	nk_glfw3_font_stash_begin(&glfw, &atlas);
	if (!(roboto = nk_font_atlas_add_from_file(atlas, FONT_PATH, 32, NULL))) {
		fprintf(stderr, "[NUKLEAR_FONT] Couldn\'t load font %s\n", FONT_PATH);
		exit(1);
	}
	nk_glfw3_font_stash_end(&glfw);
	nk_style_set_font(ctx, &roboto->handle); //Set font to ordinary roboto.ttf
	//---------------------------------
	//Load images
	power[0] = img_load(POWER_OFF_ICON_PATH);
	power[1] = img_load(POWER_ON_ICON_PATH);
	restart = img_load(RESTART_ICON_PATH);
	for (int k = 0; k < 10; k++) {
		char buff[STRING_SIZE];
		sprintf(buff, CIRCLES_PATH, k);
		circle[k] = img_load(buff);
		sprintf(buff, CIRCLES_ACTIVE_PATH, k);
		circle_active[k] = img_load(buff);
	}
	//---------------------------------
	//Button Styles
	//-----------------------------------------------------------
	//Active status
	memcpy(&status_active, &ctx->style.button, sizeof(struct nk_style_button));
	status_active.normal.data.color = c_darkyellow;
	status_active.hover.data.color = c_darkyellow;
	status_active.active.data.color = c_darkyellow;
	status_active.text_hover = nk_black;
	status_active.text_normal = nk_black;
	status_active.text_active = nk_black;
	//-----------------------------------------------------------
	//Deactive status
	memcpy(&status_deactive, &ctx->style.button, sizeof(struct nk_style_button));
	status_deactive.normal.data.color = c_gray;
	status_deactive.hover.data.color = c_gray;
	status_deactive.active.data.color = c_gray;
	//-----------------------------------------------------------
	//Border Only
	memcpy(&border_only, &ctx->style.button, sizeof(struct nk_style_button));
	border_only.normal.data.color = nk_black;
	border_only.hover.data.color = nk_black;
	border_only.active.data.color = nk_black;
	border_only.border_color = c_lightgray;
	border_only.border = 2; //2pt border width
	//-----------------------------------------------------------
	//No_Border
	memcpy(&no_border, &ctx->style.button, sizeof(struct nk_style_button));
	no_border.normal.data.color = nk_black;
	no_border.hover.data.color = nk_black;
	no_border.active.data.color = nk_black;
	no_border.border = 0; // No border, indeed
	//-----------------------------------------------------------
	//Clock pulse indicator
	memcpy(&cp, &ctx->style.button, sizeof(struct nk_style_button));
	cp.normal.data.color = nk_black;
	cp.hover.data.color = nk_black;
	cp.active.data.color = nk_black;
	cp.border = 0; //Disable border
	cp.text_hover = nk_white;
	cp.text_normal = nk_white;
	cp.text_active = nk_white;
	///-----------------------------------------------------------
	//Knob styles (hide knob's graphics)
	ctx->style.knob.knob_normal = nk_black;
	ctx->style.knob.knob_active = nk_black;
	ctx->style.knob.knob_hover = nk_black;
	ctx->style.knob.border = 0;
	ctx->style.knob.knob_border = 0;
	ctx->style.knob.cursor_width = 0;
	ctx->style.knob.cursor_active = nk_black;
	ctx->style.knob.cursor_normal = nk_black;
	ctx->style.knob.cursor_hover = nk_black;
	//---------------------------------
	//Initialize register cache
	CPU_Registers_Cache.R_PC = CPU_Registers->R_PC;
	CPU_Registers_Cache.R_AC = CPU_Registers->R_AC;
	//---------------------------------
	//GUI Infinite loop :P
	while(!glfwWindowShouldClose(main_win) && (p_reset_timer_start || CPU_Registers->p_id_trigger || is_parent_alive(hParentProcess))) { //Close button
		if (((current = glfwGetTime()) - previous) > REGISTER_LABELS_UPDATE_RATE) { //For optimization reasons, do the following each REGISTER_LABEL_UPDATE_RATE seconds
			previous = current;
			//Check for changes and reevaluate strings if necessary
			if (CPU_Registers->R_PC != CPU_Registers_Cache.R_PC) {
				sprintf(S_PC, "%04d", CPU_Registers->R_PC); //Program Counter
				CPU_Registers_Cache.R_PC = CPU_Registers->R_PC;
			}
			if (CPU_Registers->R_AC != CPU_Registers_Cache.R_AC) {
				sprintf(S_AC, "%04ll", CPU_Registers->R_AC);
				CPU_Registers_Cache.R_AC = CPU_Registers->R_AC;
			}
			//Convert CPU Register values to strings
			if (CPU_Registers->step == DECODE) {
				*S_IR = '\0';
				for (int i = 0; i <= CPU_Registers->R_IR_INST.inst_params; i++) {
					int s_ir_len;
					if (!i)
						strcat(S_IR, CPU_Registers->R_IR_INST.inst_name);
					else
						strcat(S_IR, CPU_Registers->R_IR[i - 1]);
					s_ir_len = strlen(S_IR);
					S_IR[s_ir_len] = ' ';
					S_IR[s_ir_len + 1] = '\0';
				}
			}
		}
		if (CPU_Registers->p_id_trigger) {
			if (!p_reset_timer_start)
				p_reset_timer_start = glfwGetTime();
			p_reset_timer = glfwGetTime() - p_reset_timer_start;
			if (p_reset_timer > PROCESS_RESTART_INTERVAL) {
				CPU_Registers->p_id_trigger = false;
				hParentProcess = OpenProcess(SYNCHRONIZE, false, CPU_Registers->p_id);
				p_reset_timer_start = 0;
			}	
		}
		indicators_eval(state_indicators, status_deactive, status_active); //Evaluate status indicators
		nk_glfw3_new_frame(&glfw); //Render a new frame on each iteration of the loop
		if (nk_begin(ctx, "", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) { // Main Window
			//---------------------------------
			//First Row: Status Indicators & The power button
			nk_layout_row_begin(ctx, NK_DYNAMIC, 64, 4);
			nk_layout_row_push(ctx, 0.1f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.266f);
			nk_button_label_styled(ctx, &state_indicators[FETCH], "FETCH");
			nk_layout_row_push(ctx, 0.266f);
			nk_button_label_styled(ctx, &state_indicators[DECODE], "DECODE");
			nk_layout_row_push(ctx, 0.266f);
			nk_button_label_styled(ctx, &state_indicators[EXECUTE], "EXECUTE");
			nk_layout_row_end(ctx);
			//Placeholder
			nk_layout_row_dynamic(ctx, 10, 1);
			//---------------------------------
			//Second Row: Instruction Register label
			nk_layout_row_begin(ctx, NK_DYNAMIC, 40, 2);
			nk_layout_row_push(ctx, 0.25f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.5f);
			nk_label(ctx, "Instruction Register", NK_TEXT_CENTERED);
			nk_layout_row_end(ctx);
			nk_layout_row_dynamic(ctx, 5, 1);	//Placeholder
			//---------------------------------
			//Third Row: Instruction Register value
			nk_layout_row_begin(ctx, NK_DYNAMIC, 60, 2);
			nk_layout_row_push(ctx, 0.1f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.8f);
			nk_button_label_styled(ctx, &border_only, S_IR);
			nk_layout_row_end(ctx);
			nk_layout_row_dynamic(ctx, 25, 2); //Placeholder
			//---------------------------------
			//Fourth Row: Clock
			nk_layout_row_begin(ctx, NK_DYNAMIC, 200, 2);
			nk_layout_row_push(ctx, 0.3f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.4f);
			if (nk_knob_int(ctx, 0, &knobState, 9, 1, NK_DOWN, 60)) { //Reevaluate frequency values if the knob is touched
				CPU_Registers->cp_differ_toggle = true;
				clock_freq = clock_freq_unit * (knobState + 1);
				CPU_Registers->CPU_Clock = 1000 / clock_freq;
				humanize_freq(S_CPUClock, clock_freq);
			}
			if (CPU_Registers->cp_toggle)
				nk_draw_image(nk_window_get_canvas(ctx), knob_image_bounds, &circle_active[knobState], nk_white);
			else
				nk_draw_image(nk_window_get_canvas(ctx), knob_image_bounds, &circle[knobState], nk_white);
			nk_draw_text(nk_window_get_canvas(ctx), knob_text_bounds, S_CPUClock, strlen(S_CPUClock), &roboto->handle, nk_black, nk_white);
			nk_layout_row_dynamic(ctx, 25, 1); //Placeholder
			//---------------------------------
			//Fifth Rows: PC & AC
			//PC
			nk_layout_row_begin(ctx, NK_DYNAMIC, 60, 3);
			nk_layout_row_push(ctx, 0.15f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.35f);
			nk_button_label_styled(ctx, &border_only, "PC");
			nk_layout_row_push(ctx, 0.35f);
			nk_button_label_styled(ctx, &border_only, S_PC);
			nk_layout_row_end(ctx);
			nk_layout_row_dynamic(ctx, 5, 1); //Placeholder
			//AC
			nk_layout_row_begin(ctx, NK_DYNAMIC, 60, 3);
			nk_layout_row_push(ctx, 0.15f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.35f);
			nk_button_label_styled(ctx, &border_only, "AC");
			nk_layout_row_push(ctx, 0.35f);
			nk_button_label_styled(ctx, &border_only, S_AC);
			nk_layout_row_end(ctx);
			//---------------------------------
			//Sixth Row: Debug menu toggle & power button
			nk_layout_row_dynamic(ctx, 5, 1); //Placeholder
			nk_layout_row_begin(ctx, NK_DYNAMIC, 60, 4);
			nk_layout_row_push(ctx, 0.133f);
			nk_label(ctx, "", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, 0.5f);
			if (nk_button_label(ctx, watchMenuToggleBuffer[watchMenu])) {
				watchMenu = !watchMenu; //Toggle menu if pressed
				if (watchMenu) //Change the window size accordingly
					glfwSetWindowSize(main_win, 2 * WINDOW_WIDTH, WINDOW_HEIGHT);
				else
					glfwSetWindowSize(main_win, WINDOW_WIDTH, WINDOW_HEIGHT);
			}
			//Power Button
			nk_layout_row_push(ctx, 0.117f);
			if (nk_button_image_styled(ctx, &no_border, power[CPU_Registers->power_state]))
				CPU_Registers->power_state = !CPU_Registers->power_state;
			nk_layout_row_push(ctx, 0.117f);
			if (nk_button_image_styled(ctx, &no_border, restart) && CPU_Registers->power_state) {
				knobState = 0;
				clock_freq = clock_freq_unit * (knobState + 1);
				humanize_freq(S_CPUClock, clock_freq);
				CPU_Registers->restart_trigger = true;
			}

			nk_layout_row_end(ctx);
		}
		nk_end(ctx);
		//Second Window: Debug Menu
		if (watchMenu) {
			char label[STRING_SIZE];
			const char *viewTypes[] = {"%lld", "0x%06x", "%c"}; //Register view formattings (decimal, hex, char)
			int watchMenuMaxRows = reg_stack_top >= WATCH_MENU_ROWS_DEFAULT ? reg_stack_top : WATCH_MENU_ROWS_DEFAULT; //No Scrollbar for watches < WATCH_MENU_ROWS_DEFAULT
			if (nk_begin(ctx, "Debug", nk_rect(WINDOW_WIDTH, 0, WINDOW_WIDTH, WINDOW_HEIGHT), NK_WINDOW_BORDER | NK_WINDOW_BACKGROUND)) { //Second window: debug menu
				for (int label_count = 0; label_count < watchMenuMaxRows; label_count++) {
					nk_layout_row_dynamic(ctx, 73, 2);
					if (label_count >= reg_stack_top) //Empty row if nothing is being watched
						continue;
					sprintf(label, viewTypes[registers[label_count].viewType], *registers[label_count].p);
					if (nk_widget_is_hovered(ctx))
						nk_tooltip(ctx, "Click to Copy, Right Click to Remove");
					if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_RIGHT, nk_widget_bounds(ctx), true))
						regWatch_uninit(label_count);
					if (nk_button_label_styled(ctx, &border_only, registers[label_count].name)) {
						glfwSetClipboardString(main_win, label);
						copiedAlert = true;
					}
					if (nk_widget_is_hovered(ctx))
						nk_tooltip(ctx, "Click to Copy, Right Click to Remove");
					if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_RIGHT, nk_widget_bounds(ctx), true))
						regWatch_uninit(label_count);
					if (nk_button_label_styled(ctx, &border_only, label)) {
						glfwSetClipboardString(main_win, label);
						copiedAlert = true;
					}
				}
				nk_layout_row_dynamic(ctx, 60, 1);
				if (nk_button_label(ctx, "Add Watch") && !successErrorWindow)
					addMenu = true;
				nk_end(ctx);
			}
		}
		if (addMenu && watchMenu) { //Add watch
			int selectedViewType;

			const char *viewTypes[] = {"Decimal", "Hexadecimal", "Character"};
			if (nk_begin(ctx, "Add Watch", nk_rect(600, 350, 400, 250), NK_FLAGS_ALERT_WINDOW)) { //Third window: add watch
				nk_layout_row_dynamic(ctx, 37.5f, 1);
				nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, reg_id, STRING_SIZE, nk_filter_default); //Textbox
				nk_layout_row_dynamic(ctx, 2, 1);
				nk_layout_row_dynamic(ctx, 37.5f, 1);
				nk_combobox(ctx, viewTypes, 3,  &selectedViewType, 32, nk_vec2(350, 180)); //combobox (display formatting)
				nk_layout_row_dynamic(ctx, 42, 1);
				nk_layout_row_dynamic(ctx, 50, 2);
				if (nk_button_label(ctx, "Submit")) {
					if (regWatch_init(selectedViewType, reg_id)) {
						addMenu = false;
						successErrorWindow = REG_SUCCESS; //Success window if successful
					} else {
						successErrorWindow = REG_ERROR; //Error window if not =(
					}
				}
				if (nk_button_label(ctx, "Cancel")) {
					*reg_id = '\0'; //Clear textbox buffer after the submit button is pressed for the first time..
					addMenu = false;
				}

				nk_end(ctx);
			}
		}
		if (successErrorWindow && watchMenu) { //Error, success window
			if (nk_begin(ctx, successErrorWindow == REG_SUCCESS ? "Success" : "Error", nk_rect(650, 256, 300, 170), NK_FLAGS_ALERT_WINDOW)) { //Title
				nk_layout_row_dynamic(ctx, 50, 1);
				if (successErrorWindow == REG_ERROR)
					nk_label_colored(ctx, "Error: Not Found", NK_TEXT_CENTERED, nk_red);
				else {
					sprintf(temp_buffer, "Success: Watching %s", reg_id);
					nk_label_colored(ctx, temp_buffer, NK_TEXT_CENTERED, nk_green);
				}
				nk_layout_row_dynamic(ctx, 50, 1);
				if (nk_button_label(ctx, "OK")) {
					successErrorWindow = REG_DISABLED;
					*reg_id = '\0'; //Clear textbox buffer after the submit button is pressed for the first time..
				}
				nk_end(ctx);
			}
		}
		if (copiedAlert && watchMenu) { //Displayed when copying register's value to clipboard
			if (nk_begin(ctx, "Clipboard", nk_rect(650, 356, 300, 170), NK_FLAGS_ALERT_WINDOW)) { //Title
				nk_layout_row_dynamic(ctx, 50, 1);
				strcpy(temp_buffer, "Copied Successfully");
				nk_label_colored(ctx, temp_buffer, NK_TEXT_CENTERED, nk_green);
				nk_layout_row_dynamic(ctx, 50, 1);
				if (nk_button_label(ctx, "OK")) {
					copiedAlert = false;
				}
				nk_end(ctx);
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
		glfwSwapBuffers(main_win);
		glfwPollEvents();
	}
	gui_shutdown(&glfw, main_win, hParentProcess);
}
