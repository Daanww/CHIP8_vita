#ifndef CPU_H
#define CPU_H

typedef struct {
	unsigned char RAM[0xFFF];
	unsigned short registers[0x10];

	unsigned short indexRegister;
	unsigned short programCounter;

	unsigned short stack[32];
	short stackTop;
	unsigned short stackTopAddress;

	//timers
	short delayTimer;
	short soundTimer;

	bool isPaused;

} machine;


#endif // !CPU_H
