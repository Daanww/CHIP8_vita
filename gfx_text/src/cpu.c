#include <psp2/kernel/processmgr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "cpu.h"
#include "debugScreen.h"

const static char fontArray[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};



bool isStackEmpty(machine* cpu)
{
	if ((cpu->stackTop) == -1)
		return true;
	else
	{
		return false;
	}
}
bool isStackFull(machine* cpu)
{
	if ((cpu->stackTop) == (STACK_SIZE - 1))
		return true;
	else
	{
		return false;
	}
}
void pushStack(machine* cpu, unsigned short address)
{
	if (!isStackFull(cpu))
	{
		(cpu->stackTop)++;
		cpu->stack[(cpu->stackTop)] = address;
		(cpu->stackTopAddress) = address;
	}
	else
	{
		DebugPrintSDL("Couldnt push on stack, stack is full!", 3);
	}
}
unsigned short popStack(machine* cpu)
{
	unsigned short address;
	if (!isStackEmpty(cpu))
	{
		address = cpu->stack[(cpu->stackTop)];
		(cpu->stackTop)--;
		(cpu->stackTopAddress) = cpu->stack[(cpu->stackTop)];
		return address;
	}
	else
	{
		DebugPrintSDL("Could not pop data from stack, stack is empty!", 3);
	}
}


int updateTimers(machine* cpu)
{
	(cpu->delayTimer)--;
	if ((cpu->delayTimer) < 0)
		(cpu->delayTimer) = 60;

	(cpu->soundTimer)--;
	if ((cpu->soundTimer) < 0)
		(cpu->soundTimer) = 60;

	return SDL_GetTicks();
}

void loadRom(machine* cpu,char* romName)
{
	int c, i, max;

	char filepath[64] = { 0 };
	sprintf(filepath, "app0:roms/%s", romName);

	FILE* fp = fopen(filepath, "rb");
	if (fp == NULL)
		DebugPrintSDL("Cant find/open rom!", 3);

	for (i = 0x200, max = 0x1000; i < max && (c = getc(fp)) != EOF; i++)
	{
		cpu->memory[i] = c;
	}



	fclose(fp);
}

void clearRom(machine* cpu)
{
	for (int i = 0x200, max = 0x1000; i < max; i++)
	{
		cpu->memory[i] = 0;
	}
}

void loadFont(machine* cpu)
{
	//load font into memory
//stores font from 0x050-0x09F
	for (int i = 0; i < (sizeof(fontArray) / sizeof(char)); i++)
	{
		cpu->memory[0x50 + i] = fontArray[i];
	}
}

void clearMemory(machine* cpu)
{
	for (int i = 0x000, max = 0x1000; i < max; i++)
	{
		cpu->memory[i] = 0;
	}
}

void clearScreen(machine* cpu)
{
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			cpu->pixelArray[i][j] = false;
		}
	}
}

void processIntructions(int n, machine* cpu)
{

}