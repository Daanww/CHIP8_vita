#ifndef TEXT_H
#define TEXT_H


enum {
	TTF = 1,
	SDL = 2
};


struct textData {
	char text[32];
	int size;
	SDL_Color textColor;
	//coordinates
	int x;
	int y;
	SDL_Texture* textTexture;
	bool hasNumValue;
	unsigned short numValue;
	//offset from text coordinates to render at
	int xOffset;
	int yOffset;
	SDL_Color numColor;
	SDL_Texture* numTexture;
};



//array which holds all the data for rendering text elements to screen (memory, registers, stackpointer, timers, current instruction etc.)
//another array is also created holding pointers to the numerical values associated with those text elements.
//They contain the same text in the same order (eg. textDataArray[i] contains a text element of which the value is pointed to by a pointer in textPointerArray[i].
//If a text element does not contain a value (eg. is not a number) then the value will be left as 0.
//This second array is used to check when values of numbers change to make sure the textures get updated.
extern struct textData textDataArray[64];
extern unsigned short* textPointerArray[64];

extern SDL_Rect tempTextureRect;
extern TTF_Font *currentFont;


char *hexToString(short hex);
SDL_Texture* loadText(SDL_Renderer *renderer, const char *text, int size, SDL_Color color);

void addToTextArrays(struct textData *textData, short *value);

void updateTextElementsNumValues();
void destroyTextTextures();

#endif
