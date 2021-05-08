#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>


#include "text.h"
#include <debugScreen.h>

//array which holds all the data for rendering text elements to screen (memory, registers, stackpointer, timers, current instruction etc.)
//another array is also created holding pointers to the numerical values associated with those text elements.
//They contain the same text in the same order (eg. textDataArray[i] contains a text element of which the value is pointed to by a pointer in textPointerArray[i].
//If a text element does not contain a value (eg. is not a number) then the value will be left as 0.
//This second array is used to check when values of numbers change to make sure the textures get updated.
struct textData textDataArray[64] = { 0 };
unsigned short* textPointerArray[64] = { [0 ... 63] = NULL };

//used for rendering text elements
SDL_Rect tempTextureRect = { 0, 0, 0, 0 };

//pointer to the current font being used
TTF_Font *currentFont = NULL;



//convert a number to a string in hex representation
char *hexToString(unsigned short hex)
{
	static char string[8];
	int nChar = sprintf(string, "%#04X", hex);
	//changing the x to lowercase to for aestetic reasons
	string[1] = 'x';
	return string;
}

// Function for loading font and drawing text into SDL_Texture
SDL_Texture* loadText(SDL_Renderer *renderer, const char *text, int size, SDL_Color color)
{
	static int lastSize = 0;
	//check if new text size is needed and opening new font for that
	if(lastSize != size)
	{
		lastSize = size;
		currentFont = TTF_OpenFont("/sce_sys/font/Roboto-Regular.ttf", size);
		if (currentFont == NULL)
		{
			DebugPrint("TTF_OpenFont", TTF);
			return NULL;
		}
	}

	SDL_Surface *surface = TTF_RenderText_Solid(currentFont, text, color);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	if (texture == NULL)
	{
		DebugPrint("SDL_CreateTextureFromSurface", SDL);
		return NULL;
	}
	return texture;
}


//adding text elements to textDataArray and textPointerArray
void addToTextArrays(struct textData *textData, unsigned short *value)
{

	for (int i = 0; i < 63; i++)
	{
		//checking for the first "empty" slot in the array
		if (textDataArray[i].size == 0)
		{
			textDataArray[i] = *textData;
			if (textData->hasNumValue)
			{
				textPointerArray[i] = value;
			}
			break;
		}
		else
		{
			if (i == 63)
			{
				DebugPrint("Cant add text to arrays, arrays are full", 3);
				break;
			}
		}
		
		
	}
}

void updateTextElementsNumValues()
{
	//check if numValue of a text element has changed and update it if necessary
	for (int i = 0; i < 63; i++)
	{
		//check if struct slot is even being used and if it has a numValue
		if ((textDataArray[i].size != 0) && textDataArray[i].hasNumValue)
		{
			//check if numValue is different from the actual value it represents
			if (textDataArray[i].numValue != *(textPointerArray[i]))
			{
				textDataArray[i].numValue = *(textPointerArray[i]);
				createTextTexture(&(textDataArray[i]), false, true);
			}
		}
	}
}

void destroyTextTextures()
{
	for (int i = 0; i < 63; i++)
	{
		//check if struct slot is even being used and if it has a numValue
		if (textDataArray[i].size != 0)
		{
			SDL_DestroyTexture(textDataArray[i].textTexture);
			if (textDataArray[i].hasNumValue)
			{
				SDL_DestroyTexture(textDataArray[i].numTexture);
			}
		}
	}

}

