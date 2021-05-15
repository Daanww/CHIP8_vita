#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "debugScreen.h"
#include "text.h"
#include "cpu.h"
#include "input.h"

#define printf psvDebugScreenPrintf


const static char romNames[4][32] = 
{
	"BC_test.ch8",
	"IBM_Logo.ch8",
	"test_opcode.ch8",
	"Space_invaders.ch8"
};
enum
{
	BC_test, IBM_Logo, test_opcode, Space_invaders
};
const static int currentRom = BC_test;

//some colors I got from colorhunt.co
SDL_Color color_white = { 0xEA, 0xEA, 0xEA };		// #eaeaea
SDL_Color color_lightBlue = { 0x08, 0xD9, 0xD6 };	// #08D9D6
SDL_Color color_red = { 0xFF, 0x2E, 0x63 };			// #FF2E63
SDL_Color color_black = { 0x25, 0x2A, 0x34 };		// #252A34

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int textSize = 20;



//opens debugwindow and prints error message
void DebugPrintSDL(const char function[],int errorType)
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

//opens debugwindow and prints error message
void DebugPrintValue(const char function[], int value)
{
	int remain = 10;
	psvDebugScreenInit();
	
	printf("Potential error: %s, value: %X", function, value);

	while (remain-- > 0)
	{
		sceKernelDelayThread(1000 * 1000); //delay 1 second
	}


}

int romSelect(keypadPresses* input)
{
	int romSelect = 0;
	psvDebugScreenInit();
	keypadPresses oldInput;
	while (input->keys[7] == false)
	{
		oldInput = *input;
		getInput(input);
		if (input->keys[2] && !(oldInput.keys[2]))
		{
			romSelect++;
			if(romSelect > ((sizeof(romNames)/sizeof(romNames[0]) - 1)))
				romSelect--;
		}
		if (input->keys[1] && !(oldInput.keys[1]))
		{
			romSelect--;
			if (romSelect < 0)
				romSelect++;
		}
		printf("Select a rom. Use UP & DOWN to browse roms and key7 to select. ");
		printf("rom: %s\r", romNames[romSelect]);

		sceKernelDelayThread(20 * 1000);
	}
	return romSelect;
}

//draws the pixelArray, returns the time at which it draws, use this to keep frame rate locked
int DrawPixelArray(machine* cpu, SDL_Rect pixel)
{

	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numColumns; j++)
		{
			if (cpu->pixelArray[i][j] == true)
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
		DebugPrintSDL("SDL_Init", SDL);
		success = false;
	}
	if (TTF_Init() != 0)
	{
		DebugPrintSDL("TTF_Init", TTF);
		success = false;
	}

	window = SDL_CreateWindow("CHIP8EMU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 544, 0);
	if (window == NULL)
	{
		DebugPrintSDL("SDL_CreateWindow", SDL);
		success = false;
	}

	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		DebugPrintSDL("SDL_CreateRenderer", SDL);
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



int main(int argc, char *argv[])
{
	
	bool initializationFlag = initialize();
	if (!initializationFlag)
		return 1;

	//initialize input
	keypadPresses input;
	input = initializeInput();

	

	

	//create the main processor and initializing stuff
	machine cpu;
	initProcessor(&cpu, &input);
	loadRom(&cpu, romNames[romSelect(&input)]);
	
	//--- Add all text element to text arrays ---

	//a struct to initialize all the register structs with
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
		addToTextArrays(&(registertextDataArray[i]), &(cpu.registers[i]));
	}
	//bottom of the registers is at registerHeaderData.y + (textSize * 6 /5) * 9

	
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
	addToTextArrays(&indexRegisterData, &(cpu.indexRegister));



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
	addToTextArrays(&programCounterData, &(cpu.programCounter));



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
	addToTextArrays(&stackTopData, &(cpu.stackTop));

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
	addToTextArrays(&stackTopAddressData, &(cpu.stackTopAddress));



	struct textData delayTimerData = {
		.text = "delayTimer:",
		.size = textSize,
		.textColor = color_red,
		.x = 5,
		.y = 375,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.xOffset = textSize * 6,
		.yOffset = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&delayTimerData, &(cpu.delayTimer));

	struct textData soundTimerData = {
		.text = "soundTimer:",
		.size = textSize,
		.textColor = color_red,
		.x = 5,
		.y = 400,
		.textTexture = NULL,
		.hasNumValue = true,
		.numValue = 0,
		.xOffset = textSize * 6,
		.yOffset = 0,
		.numColor = color_white,
		.numTexture = NULL };
	addToTextArrays(&soundTimerData, &(cpu.soundTimer));

	struct textData currentInstructionData = {
	.text = "Instr:",
	.size = textSize,
	.textColor = color_red,
	.x = 5,
	.y = 425,
	.textTexture = NULL,
	.hasNumValue = true,
	.numValue = 0,
	.xOffset = textSize * 4,
	.yOffset = 0,
	.numColor = color_white,
	.numTexture = NULL };
	addToTextArrays(&currentInstructionData, &(cpu.currentInstruction));

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

	





	
	
	//screen is 960x544 pixels, drawing the game at a resolution of 640x320 gives enough room for debug text and allows for game pixels of either 10x10 or 5x5.
	//thus the game window will be from (160,0) to (800,320) (y=0 at top of window)



	//creating rects for game window
	SDL_Rect gameWindow[3] = { {150, 0, 10, 320}, {150, 320, 660, 10}, {800, 0, 10, 320}  };

	//setting up stuff for drawing
	SDL_Rect pixel = { 0, 0, 10, 10 };
	int lastDrawTime = 0;
	int timeBetweenDraws = 30; //time in milliseconds
	int totalFrames = 0;
	int lastUpdateTimersTime = 0;
	keypadPresses oldInput;
	bool pressTRIANGLE = false;

	//main loop
	while (true)
	{
		
		updateTextElementsNumValues();
		
		oldInput = input;
		getInput(&input);

		if (input.keys[5])
			pressTRIANGLE = true;

		if (SDL_GetTicks() >= (lastUpdateTimersTime + 15))
		{
			lastUpdateTimersTime = updateTimers(&cpu);
		}

		//drawing to screen
		if (SDL_GetTicks() >= (lastDrawTime + timeBetweenDraws))
		{
			if (input.keys[0])
				break;

			if (pressTRIANGLE)
				processInstructions(STEPS_PER_CYCLE, &cpu);

			//clearing screen
			SDL_SetRenderDrawColor(renderer, color_black.r, color_black.g, color_black.b, 255);
			SDL_RenderClear(renderer);

			//drawing game window
			SDL_SetRenderDrawColor(renderer, color_lightBlue.r, color_lightBlue.g, color_lightBlue.b, 255);
			SDL_RenderFillRect(renderer, &(gameWindow[0]));
			SDL_RenderFillRect(renderer, &(gameWindow[1]));
			SDL_RenderFillRect(renderer, &(gameWindow[2]));
			
			



			//drawing pixelArray
			SDL_SetRenderDrawColor(renderer, color_white.r, color_white.g, color_white.b, 255);			
			lastDrawTime = DrawPixelArray(&cpu, pixel);


			drawTextElements();
			

			SDL_RenderPresent(renderer);
			totalFrames++;
			

		}


		
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