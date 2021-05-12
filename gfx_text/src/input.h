#ifndef INPUT_H
#define INPUT_H

#include <psp2/ctrl.h>

typedef struct {

	SceCtrlData input;

	//maps the CHIP8 keypad to the vita inputs (represented here as an int array of size 20)
	//thus inputmapping[6] is enum 6 of vita inputs ("CROSS"), and the value it holds is the CHIP8 keypad button
	int inputMapping[20];
	//a list of the CHIP8 keys, with a boolean attached to indicate if they are pressed or not

	//!!!! if youre going to change the size of keys array, also change it in GetInput(), otherwise you will get memory corruption. current size is 16 !!!!
	bool keys[16];

} keypadPresses;

keypadPresses initializeInput();
int findKeypadKey(char* buffer, char* hashtag);
int findVitaKey(char* buffer);
void getInput(keypadPresses* keypadPresses);

#endif // !INPUT_H