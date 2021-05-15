#ifndef CPU_H
#define CPU_H
#include "input.h"

#define STACK_SIZE 16 //stack size in bytes
#define PROGRAM_START_ADDRESS 0x200 //memory location where programs will be loaded
#define SPEED 60 //timers update frequency
#define STEPS_PER_CYCLE 10 //number of steps performed in one cycle

enum {
	numRows = 32,
	numColumns = 64
};

typedef struct {
	unsigned char memory[0x1000]; //address 0x000-0xFFF
	unsigned short registers[0x10]; //registers 0-F

	unsigned short indexRegister;
	unsigned short programCounter;

	unsigned short stack[STACK_SIZE];
	short stackTop;
	unsigned short stackTopAddress;

	//array for drawing
	//false = no draw, true = draw white pixel at that location
	bool pixelArray[numRows][numColumns];

	//timers
	short delayTimer;
	short soundTimer;

	bool isPaused;

	unsigned short currentInstruction;

	bool* keys; //a reference to the keys array in input, will be used for input checking instructions. Assumed size is 17 because thats the size of the array in input at the current time

} machine;


void initProcessor(machine* cpu, keypadPresses* input);
bool isStackEmpty(machine* cpu);
bool isStackFull(machine* cpu);
void pushStack(machine* cpu, unsigned short address);
unsigned short popStack(machine* cpu);
int updateTimers(machine* cpu);
void loadRom(machine* cpu, char* romName);
void clearRom(machine* cpu);
void loadFont(machine* cpu);
void clearMemory(machine* cpu);
void clearScreen(machine* cpu);
void processInstructions(int n, machine* cpu);
void instruction_DXYN(unsigned int instruction, machine* cpu);


#endif // !CPU_H
