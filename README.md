
# Computer Simulation, Written in C

This is a simple computer simulation, with a proprietary machine code language which the programs are written in.

The project mainly consists of two seperate parts, which comminucate using windows.h's file mapping:

1. The computer itself
2. System monitor GUi
## The computer itself
### Main Components
#### CPU
- Has the registers IR, PC, SP, AC, MAR, MDR, U, V, X, Y, Z
- Reads instructions from systemMemory (Defined in Memory.c)
- Parses each instruction using a hashMap (Defined in headers.h)
- Handles the shared memory (hMapFile)
- Follows a three stepped procedure: FETCH, DECODE, EXECUTE
- Will wait a pre-defined amount of time on each step, simulating a clock frequency.
#### Memory
- Contains two seperate channels
- Each channel is a global variable defined in Memory.c
- The first channel is systemMemory, Reserved for CPU instructions
- The second channel is userMemory, which can be accesed via memory related commands, MDR, and MAR registers.
```c
typedef struct systemRAM {
	unsigned long instruction;
	char data[SYNTAX_LIMIT][STRING_SIZE];
	int type;
	bool isFree;
} systemRAM;
typedef struct userRAM {
	int data;
	bool isFree;
} userRAM;
```


