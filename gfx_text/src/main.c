#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
SDL_Color color_white = { 0xF9, 0xF7, 0xF7 };		// #f9f7f7
SDL_Color color_lightBlue = { 0xDB, 0xE2, 0xEF };	// #dbe2ef
SDL_Color color_blue = { 0x3F, 0x72, 0xAF };		// #3f72af
SDL_Color color_darkBlue = { 0x11, 0x2D, 0x4E };	// #112d4e

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;



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
				tempTextureRect.y += (textDataArray[i].size + 1);
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

	//indexRegister(points to memory adresses)
	unsigned short indexRegister = 0x0;
	struct textData indexRegisterData = {
		.text = "indexRegister",
		.size = 20,
		.textColor = color_lightBlue,
		.x = 0,
		.y = 10,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&indexRegisterData, &indexRegister);


	//program counter (points to current instruction in memory)
	unsigned short programCounter = 0x0;
	struct textData programCounterData = {
		.text = "programCounter",
		.size = 20,
		.textColor = color_lightBlue,
		.x = 0,
		.y = 50,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&programCounterData, &programCounter);

	//stack (holds memory locations for calling/returning from subroutines)
	unsigned short stack[64] = { 0x0 };
	unsigned short* stackPointer = stack;

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
	SDL_Rect gameWindow[3] = { {150, 0, 10, 320}, {800, 0, 10, 320}, {150, 320, 660, 10} };

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
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			//drawing game window
			SDL_SetRenderDrawColor(renderer, color_darkBlue.r, color_darkBlue.g, color_darkBlue.b, 255);
			SDL_RenderFillRects(renderer, &gameWindow[0], 3);

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