#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <debugScreen.h>
#include "text.h"

#define printf psvDebugScreenPrintf


const char fontArray[] = {
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

//some colors I got from colorhunt.co
SDL_Color color_white = { 0xEA, 0xEA, 0xEA };		// #eaeaea
SDL_Color color_lightBlue = { 0x08, 0xD9, 0xD6 };	// #08D9D6
SDL_Color color_red = { 0xFF, 0x2E, 0x63 };			// #FF2E63
SDL_Color color_black = { 0x25, 0x2A, 0x34 };		// #252A34

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int textSize = 20;

//opens debugwindow and prints error message
void DebugPrint(const char function[], int errorType)
{
	int remain = 10;
	psvDebugScreenInit();
	if (errorType == 1)
	{
		printf("%s is not working!, TTF Error: %s", function, TTF_GetError());
	}
	else if (errorType == 2)
	{
		printf("%s is not working!, SDL Error: %s", function, SDL_GetError());
	}
	else
	{
		printf("Other error: %s", function);
	}
	while (remain-- > 0)
	{
		sceKernelDelayThread(1000 * 1000);
	}


}



//draws the pixelArray, returns the time at which it draws, use this to keep frame rate locked
int DrawPixelArray(char pixelArray[numRows][numColumns], SDL_Rect pixel)
{

	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			if (pixelArray[i][j] == 1)
			{
				//160 = (960-640)/2, the top left corner of the game window
				pixel.x = 160 + j * pixel.w;
				pixel.y = i * pixel.h;
				SDL_RenderFillRect(renderer, &pixel);
			}
		}
	}
	int thisDrawTime = SDL_GetTicks();
	return thisDrawTime;
}

//initializes SDL and TTF. Also creates window and renderer
bool initialize()
{
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		DebugPrint("SDL_Init", SDL);
		success = false;
	}
	if (TTF_Init() != 0)
	{
		DebugPrint("TTF_Init", TTF);
		success = false;
	}

	window = SDL_CreateWindow("CHIP8EMU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 544, 0);
	if (window == NULL)
	{
		DebugPrint("SDL_CreateWindow", SDL);
		success = false;
	}

	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		DebugPrint("SDL_CreateRenderer", SDL);
		success = false;
	}
	
	return success;
}


//creating and updating the textures for text elements.
void createTextTexture(struct textData *textData, bool updateText, bool updateNum)
{
	if (updateText)
	{
		//destroying old texture if necessary
		if (textData->textTexture != NULL)
		{
			SDL_DestroyTexture(textData->textTexture);
		}
		textData->textTexture = loadText(renderer, textData->text, textData->size, textData->textColor);
	}
	if (updateNum)
	{
		if (textData->numTexture != NULL)
		{
			SDL_DestroyTexture(textData->numTexture);
		}
		textData->numTexture = loadText(renderer, hexToString(textData->numValue), textData->size, textData->numColor);
	}
}


void drawTextElements()
{
	//drawing all text elements
	for (int i = 0; i < 63; i++)
	{
		//checking if this slot in the array is taken up by a struct thats actually being used
		if (textDataArray[i].size != 0)
		{
			//draw text element using tempTextureRect
			tempTextureRect.x = textDataArray[i].x;
			tempTextureRect.y = textDataArray[i].y;
			SDL_QueryTexture(textDataArray[i].textTexture, NULL, NULL, &tempTextureRect.w, &tempTextureRect.h);
			SDL_RenderCopy(renderer, textDataArray[i].textTexture, NULL, &tempTextureRect);

			if (textDataArray[i].hasNumValue)
			{
				//draw num element using tempTextureRect
				tempTextureRect.x += textDataArray[i].xOffset;
				tempTextureRect.y += textDataArray[i].yOffset;
				SDL_QueryTexture(textDataArray[i].numTexture, NULL, NULL, &tempTextureRect.w, &tempTextureRect.h);
				SDL_RenderCopy(renderer, textDataArray[i].numTexture, NULL, &tempTextureRect);
			}
		}
		else
		{
			break;
		}
	}
}

bool isStackEmpty(short* top)
{
	if (*top == -1)
		return true;
	else
	{
		return false;
	}
}
bool isStackFull(short* top, int stackSize)
{
	if (*top == (stackSize-1))
		return true;
	else
	{
		return false;
	}
}
void pushStack(unsigned short address, unsigned short* stack, int stackSize, short* top, unsigned short* stackTopAddress)
{
	if (!isStackFull(top, stackSize))
	{
		(*top)++;
		stack[*top] = address;
		*stackTopAddress = address;
	}
	else
	{
		DebugPrint("Couldnt push on stack, stack is full!", 3);
	}
}
unsigned short popStack(unsigned short* stack, short* top, unsigned short* stackTopAdress)
{
	unsigned short adress;
	if (!isStackEmpty(top))
	{
		adress = stack[*top];
		(*top)--;
		*stackTopAdress = stack[*top];
		return adress;
	}
	else 
	{
		DebugPrint("Could not pop data from stack, stack is empty!", 3);
	}
}


int main(int argc, char *argv[])
{
	
	bool initializationFlag = initialize();
	if (!initializationFlag)
		return 1;


	//initialize array for drawing
	//0 = no draw, 1 = draw white pixel at that location

	char pixelArray[numRows][numColumns] = { 0 };




	//initialize memory, registers, stack, timers
	//also adding them to both arrays

	//4KB of ram, this is where the font and the game itself gets loaded into
	unsigned char RAM[0xFFF] = { 0x0 };

	//16 registers (0-F, with register F being flag register)
	unsigned short registers[0x10] = { 0x0 };

	//a temporary struct to initialize all the register structs with
	struct textData registertextDataArray[16] = { 0 };

	//the main register struct, all others will depend on its coordinates
	struct textData registerHeaderData = {
		.text = "Registers:",
		.size = textSize,
		.textColor = color_red,
		.x = 5,
		.y = 95,
		.textTexture = NULL,
		.hasNumValue = false,
		.xOffset = textSize,
		.yOffset = 0,
		.numValue = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&registerHeaderData, NULL);

	//creating all other register textData structs
	for (int i = 0; i < 16; i++)
	{
		
		sprintf(registertextDataArray[i].text, "%X", i);

		registertextDataArray[i].size = textSize;
		registertextDataArray[i].textColor = color_red;

		if (i < 8)
		{
			registertextDataArray[i].x = registerHeaderData.x;
		}
		else
		{
			registertextDataArray[i].x = registerHeaderData.x + textSize * 3.5;
		}

		if (i < 8)
		{
			registertextDataArray[i].y = registerHeaderData.y + (textSize * 6 / 5) * (i+1);
		}
		else
		{
			registertextDataArray[i].y = registerHeaderData.y + (textSize * 6 / 5) * (i-7);
		}

		registertextDataArray[i].textTexture = NULL;
		registertextDataArray[i].hasNumValue = true;
		registertextDataArray[i].xOffset = textSize;
		registertextDataArray[i].yOffset = 0;
		registertextDataArray[i].numValue = 0;
		registertextDataArray[i].numColor = color_white;
		registertextDataArray[i].numTexture = NULL;
		addToTextArrays(&(registertextDataArray[i]), &(registers[i]));
	}
	//bottom of the registers is at registerHeaderData.y + (textSize * 6 /5) * 9

	

	//indexRegister(points to memory adresses)
	unsigned short indexRegister = 0x0;
	struct textData indexRegisterData = {
		.text = "indexRegister",
		.size = textSize,
		.textColor = color_red,
		.x = 5,
		.y = 5,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.xOffset = 0,
		.yOffset = textSize+1,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&indexRegisterData, &indexRegister);


	//program counter (points to current instruction in memory)
	unsigned short programCounter = 0x0;
	struct textData programCounterData = {
		.text = "programCounter",
		.size = textSize,
		.textColor = color_red,
		.x = 5,
		.y = 50,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.xOffset = 0,
		.yOffset = textSize+1,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&programCounterData, &programCounter);

	//stack (holds memory locations for calling/returning from subroutines)
	unsigned short stack[32] = { 0x0 };
	short stackTop = -1;
	unsigned short stackTopAddress = 0;

	struct textData stackTopData = {
		.text = "stackTop:",
		.size = textSize,
		.textColor = color_red,
		.x = 5,
		.y = 315,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.xOffset = textSize * 5,
		.yOffset = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&stackTopData, &stackTop);

	struct textData stackTopAddressData = {
		.text = "address:",
		.size = textSize,
		.textColor = color_red,
		.x = stackTopData.x,
		.y = stackTopData.y + textSize * 6 / 5,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.xOffset = textSize * 5,
		.yOffset = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&stackTopAddressData, &stackTopAddress);


	//timers
	unsigned short delayTimer = 60;
	unsigned short soundTimer = 60;

	//create textures for all text elements
	for (int i = 0; i < 63; i++)
	{
		//checking if this slot in the array is taken up by a struct thats actually being used
		if (textDataArray[i].size != 0)
		{
			createTextTexture(&(textDataArray[i]), true, textDataArray->hasNumValue);
			
		}
		else
		{
			break;
		}
	}

	

	//load font into memory
	//stores font from 0x050-0x09F
	for (int i = 0; i < (sizeof(fontArray) / sizeof(char)); i++)
	{
		RAM[0x50 + i] = fontArray[i];
	}

	





	
	
	//screen is 960x544 pixels, drawing the game at a resolution of 640x320 gives enough room for debug text and allows for game pixels of either 10x10 or 5x5.
	//thus the game window will be from (160,0) to (800,320) (y=0 at top of window)

	//set alternating pixels to 1

	for (int row = 0; row < numRows; row++)
	{
		for (int column = 0; column < numColumns; column++)
		{
			if (row % 2 == 0 || column % 2 == 0)
				pixelArray[row][column] = 1;
		}
	}

	//creating rects for game window
	SDL_Rect gameWindow[3] = { {150, 0, 10, 320}, {150, 320, 660, 10}, {800, 0, 10, 320}  };

	//setting up stuff for drawing
	SDL_Rect pixel = { 0, 0, 10, 10 };
	int lastDrawTime = 0;
	int timeBetweenDraws = 30;
	int totalFrames = 0;
	

	//main loop
	while (true)
	{
		
		updateTextElementsNumValues();
		


		//drawing the pixels
		if (SDL_GetTicks() >= (lastDrawTime + timeBetweenDraws))
		{
			//clearing screen
			SDL_SetRenderDrawColor(renderer, color_black.r, color_black.g, color_black.b, 255);
			SDL_RenderClear(renderer);

			//drawing game window
			SDL_SetRenderDrawColor(renderer, color_lightBlue.r, color_lightBlue.g, color_lightBlue.b, 255);
			SDL_RenderFillRects(renderer, gameWindow, 3);

			//drawing pixelArray
			SDL_SetRenderDrawColor(renderer, color_white.r, color_white.g, color_white.b, 255);			
			lastDrawTime = DrawPixelArray(pixelArray, pixel);


			drawTextElements();
			

			SDL_RenderPresent(renderer);
			totalFrames++;

			//updating indexRegister to see if the texture also automatically gets updated
			if (totalFrames % 3 == 0)
				indexRegister++;
			programCounter++;
			if (totalFrames % 5 == 0)
				registers[0]++;
			registers[1]++;
			if (totalFrames % 2 == 0)
				registers[1]--;
			registers[10]++;

			
			if (totalFrames % 20 == 0)
			{
				pushStack(totalFrames, stack, 32, &stackTop, &stackTopAddress);
			}
			if (totalFrames % 30 == 0)
			{
				int i = popStack(stack, &stackTop, &stackTopAddress);
			}
			

		}
		if (totalFrames > 300)
			break;
	}


	destroyTextTextures();

	TTF_CloseFont(currentFont);
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;
	SDL_Quit();
	sceKernelExitProcess(0);
	return 0;
}