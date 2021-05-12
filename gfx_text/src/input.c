#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "debugScreen.h"

//the order of this enum and VITAKEYS array have to be the same
enum {
	UP, 
	DOWN, 
	LEFT, 
	RIGHT, 
	TRIANGLE, 
	CIRCLE, 
	CROSS, 
	SQUARE, 
	START, 
	SELECT, 
	LTRIGGER, 
	RTRIGGER, 
	LSTICKUP, 
	LSTICKDOWN, 
	LSTICKLEFT, 
	LSTICKRIGHT, 
	RSTICKUP, 
	RSTICKDOWN, 
	RSTICKLEFT, 
	RSTICKRIGHT
};
const static char *VITAKEYS[20] = { 
	"UP",
	"DOWN",
	"LEFT",
	"RIGHT",
	"TRIANGLE",
	"CIRCLE",
	"CROSS",
	"SQUARE",
	"START",
	"SELECT",
	"LTRIGGER",
	"RTRIGGER",
	"LSTICKUP",
	"LSTICKDOWN",
	"LSTICKLEFT",
	"LSTICKRIGHT",
	"RSTICKUP",
	"RSTICKDOWN",
	"RSTICKLEFT",
	"RSTICKRIGHT" };
const static char* KEYPADKEYS[16] =
{
	"KEY_0",
	"KEY_1",
	"KEY_2",
	"KEY_3",
	"KEY_4",
	"KEY_5",
	"KEY_6",
	"KEY_7",
	"KEY_8",
	"KEY_9",
	"KEY_A",
	"KEY_B",
	"KEY_C",
	"KEY_D",
	"KEY_E",
	"KEY_F"
};

keypadPresses initializeInput()
{
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	keypadPresses keypadPresses;

	//open input.txt and setup inputMapping array in keypadPresses
	FILE* fp = fopen("app0:config/input.txt", "r");
	if (fp == NULL)
		DebugPrintSDL("Cannot open input.txt", 3);
	const int bufferSize = 512;
	char buffer[bufferSize];
	
	
	//write contents of input.txt to buffer
	while (fgets(buffer, bufferSize, fp) != NULL)
	{
		//check if line has comment
		char* hashtag = strchr(buffer, '#');
		//automatically skipping line if hashtag is first character
		if (hashtag != &buffer)
		{
			//set hashtag to end of string if it doesnt exist. this makes it possible to safely read all text in front of the hashtag pointer for this line.
			if (hashtag == NULL)
				hashtag == strchr(buffer, "\n");
			//map the found keypadkey to the found vitakey
			keypadPresses.inputMapping[findVitaKey(buffer)] = findKeypadKey(buffer, hashtag);

		}
	}
	

	fclose(fp);

	return keypadPresses;
}

//find the keypadkey on the current line
int findKeypadKey(char *buffer, char *hashtag)
{
	int keypadKey;
	
	for (keypadKey = 0; keypadKey < 16; keypadKey++)
	{
		
		if (strstr(buffer, KEYPADKEYS[keypadKey]) != NULL)
		{
			//DebugPrintValue("found a keypadkey!", keypadKey);
			return keypadKey;
		}
	}
	DebugPrintSDL("KeypadKey (1,2,3..) not found in input.txt!", 3);
	return 0x10;
}

//find the vita input on the current line
int findVitaKey(char* buffer)
{
	int vitaKey;
	for (vitaKey = 19; vitaKey >= 0; vitaKey--)
	{
		if (strstr(buffer, VITAKEYS[vitaKey]) != NULL)
		{
			//DebugPrintValue("found a vita key!", vitaKey);
			return vitaKey;
		}
	}
	DebugPrintSDL("vitaKey (UP, DOWN, LEFT...) not found in input.txt!", 3);
	return 0x10;
}


void getInput(keypadPresses* keypadPresses)
{
	memset(keypadPresses->keys, 0x00, 16);
	SceCtrlData ctrl = {0};
	sceCtrlReadBufferPositive(0, &ctrl, 1);
	if (ctrl.buttons & (SCE_CTRL_UP))
	{
		keypadPresses->keys[keypadPresses->inputMapping[0]] = true;
	}

}
