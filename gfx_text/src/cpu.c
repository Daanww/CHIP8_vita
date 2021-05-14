#include <psp2/kernel/processmgr.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

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


void initProcessor(machine* cpu)
{
	clearScreen(cpu);
	clearMemory(cpu);
	loadFont(cpu);
	cpu->programCounter = 0x200;
	cpu->indexRegister = 0x000;
	for (int i = 0; i < 16; i++)
	{
		cpu->registers[i] = 0x00;
	}
	cpu->stackTop = -1;
	cpu->stackTopAddress = 0x000;

	cpu->delayTimer = 60;
	cpu->soundTimer = 60;
	srand(time(NULL));
}

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

void processInstructions(int n, machine* cpu)
{
	unsigned int instruction = 0;
	
	

	for (int i = 0; i < n; i++)
	{
		//fetch instruction
		instruction = cpu->memory[cpu->programCounter];
		cpu->programCounter++;
		instruction = instruction << 8;
		instruction = instruction | cpu->memory[cpu->programCounter];
		cpu->programCounter++;

		cpu->currentInstruction = instruction;
		

		//decode and execute instruction
		switch (instruction & 0xF000)
		{
		case 0x0000:
			if (instruction == 0x00E0)
			{
			//00E0 clear screen
			clearScreen(cpu);
			break;
			}

			if (instruction == 0x00EE)
			{
				//00EE return from subroutine
				cpu->programCounter = popStack(cpu);
				break;
			}

		case 0x1000:
			//1NNN jump to address NNN
			cpu->programCounter = (instruction & 0x0FFF);
			break;

		case 0x2000:
			//2NNN call subroutine at address NNN
			pushStack(cpu, cpu->programCounter);
			cpu->programCounter = (instruction & 0x0FFF);
			break;

		case 0x3000:
			//3XNN skip one instruction if VX == NN
			if (cpu->registers[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF))
			{
				cpu->programCounter += 2;
			}
			break;

		case 0x4000:
			//4XNN skip one instruction if VX != NN
			if (cpu->registers[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF))
			{
				cpu->programCounter += 2;
			}
			break;

		case 0x5000:
			//5XY0 skip one instruction if VX == VY
			if(cpu->registers[(instruction & 0x0F00) >> 8] == cpu->registers[(instruction & 0x00F0) >> 4])
			{
				cpu->programCounter += 2;
			}
			break;

		case 0x6000:
			
			//6XNN set register VX to value NN
			cpu->registers[(instruction & 0x0F00) >> 8] = (instruction & 0x0FF);
			break;

		case 0x7000:
			//7XNN add value NN to register VX, this does not affect carry flag
			cpu->registers[(instruction & 0x0F00) >> 8] += (instruction & 0x0FF);
			break;

		case 0x8000:
			//logic and arithmatic instructions

			switch (instruction & 0x000F)
			{
			case 0x0:
				//8XY0 set VX to VY (VX = VY)
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x00F0) >> 4];
				break;

			case 0x1:
				//8XY1 set VX to VX OR VY
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x0F00) >> 8] | cpu->registers[(instruction & 0x00F0) >> 4];
				break;

			case 0x2:
				//8XY2 set VX to VX AND VY
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x0F00) >> 8] & cpu->registers[(instruction & 0x00F0) >> 4];
				break;

			case 0x3:
				//8XY3 set VX to VX XOR VY
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x0F00) >> 8] ^ cpu->registers[(instruction & 0x00F0) >> 4];
				break;

			case 0x4:
				//8XY4 set VX to VX + VY, this does affect carry flag
				if ((cpu->registers[(instruction & 0x0F00) >> 8] + cpu->registers[(instruction & 0x00F0) >> 4]) > 255)
				{
					cpu->registers[0xF] = 1;
				}
				else
				{
					cpu->registers[0xF] = 0;
				}
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x0F00) >> 8] + cpu->registers[(instruction & 0x00F0) >> 4];
				break;

			case 0x5:
				//8XY5 set VX to VX - VY 
				//if VX > VY, VF = 1 else, VF = 0
				if (cpu->registers[(instruction & 0x0F00) >> 8] > cpu->registers[(instruction & 0x00F0) >> 4])
				{
					cpu->registers[0xF] = 1;
				}
				else
				{
					cpu->registers[0xF] = 0;
				}
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x0F00) >> 8] - cpu->registers[(instruction & 0x00F0) >> 4];
				break;

			case 0x6:
				//8XY6 set VX to VY and then shift VX one bit to right, set VF to the bit that was shifted out
				//in modern interpreters (90s onward) this instruction is a bit different with it ignoring VY and just shifting VX. This can cause problems with programs
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x00F0) >> 4];
				cpu->registers[0xF] = (cpu->registers[(instruction & 0x0F00) >> 8] & 0x1);
				cpu->registers[(instruction & 0x0F00) >> 8] >> 1;
				break;

			case 0x7:
				//8XY7 set VX to VY - VX
				//if VY > VX, VF = 1 else, VF = 0
				if (cpu->registers[(instruction & 0x00F0) >> 4] > cpu->registers[(instruction & 0x0F00) >> 8])
				{
					cpu->registers[0xF] = 1;
				}
				else
				{
					cpu->registers[0xF] = 0;
				}
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x00F0) >> 4] - cpu->registers[(instruction & 0x0F00) >> 8];
				break;

			case 0xE:
				//8XYE set VX to VY and then shift VX one bit to left, set VF to the bit that was shifted out
				//in modern interpreters (90s onward) this instruction is a bit different with it ignoring VY and just shifting VX. This can cause problems with programs
				cpu->registers[(instruction & 0x0F00) >> 8] = cpu->registers[(instruction & 0x00F0) >> 4];
				cpu->registers[0xF] = (cpu->registers[(instruction & 0x0F00) >> 8] & 0x80);
				cpu->registers[(instruction & 0x0F00) >> 8] << 1;
				break;
			}

		case 0x9000:
			//9XY0 skip one instruction if VX != VY
			if (cpu->registers[(instruction & 0x0F00) >> 8] != cpu->registers[(instruction & 0x00F0) >> 4])
			{
				cpu->programCounter += 2;
			}
			break;

		case 0xA000:
			//ANNN set indexregister to value NNN
			cpu->indexRegister = (instruction & 0x0FFF);
			break;

		case 0xB000:
			//BNNN set programcounter to NNN + V0
			//in modern implementations it is changed to BXNN where it will jump to XNN + VX
			//this can cause problems with programs
			cpu->programCounter = ((instruction & 0x0FFF) + cpu->registers[0x0]);
			break;

		case 0xC000:
			//CXNN generates random 8 bit number then AND with NN and put result in VX
			unsigned char randValue = rand() % 0x100;
			cpu->registers[(instruction & 0x0F00) >> 8] = (randValue & (instruction & 0x00FF));
			break;

		case 0xD000:
			instruction_DXYN(instruction, cpu);
			break;

		default:
			DebugPrintValue("Unknown instruction!", instruction);
		}
	}


}


void instruction_DXYN(unsigned int instruction, machine* cpu)
{

	//DXYN draw sprite (from memory location pointed at by indexregister) at coordinates stored in registers VX and VY with height N
			//thus X corresponds to which register stores the value for the x-axis and Y for the register which stores the value for the y-axis

	unsigned char x_base = 0;
	x_base = cpu->registers[(instruction & 0x0F00) >> 8];

	unsigned char y_base = 0;
	y_base = cpu->registers[(instruction & 0x00F0) >> 4];

	unsigned char height = (instruction & 0x000F);
	
	unsigned char xcoordinate = 0;
	unsigned char ycoordinate = 0;
	
	

	

	//instruction sets the F register to 0, it will be set to 1 if any pixels on the screen were "turned off" by this operation
	//when a sprite gets draw the pixels of the sprite are XOR'ed with the pixels on screen, thus it can happen that pixels are turned off
	cpu->registers[0xF] = 0;

	int spriteWidth = 8;
	unsigned int spriteData = 0;
	for (int row = 0; row < height; row++)
	{
		
			//for if the ycoordinate gets wrapped
			ycoordinate = (y_base + row) % numRows;

		spriteData = cpu->memory[(cpu->indexRegister) + row];
		for (int col = 0; col < spriteWidth; col++)
		{
				xcoordinate = (x_base + col) % numColumns;

				
			//checking spriteData bit by bit
			if ((spriteData & (0x80 >> col)) > 0)
			{
				if (cpu->pixelArray[ycoordinate][xcoordinate] == 0)
				{
					cpu->pixelArray[ycoordinate][xcoordinate] = 1;
					
				}
				else
				{
					cpu->pixelArray[ycoordinate][xcoordinate] = 0;
					cpu->registers[0xF] = 1;
				}
			}
		}
	}
}