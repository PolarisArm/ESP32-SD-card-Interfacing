#include <Arduino.h>
#include <TFT_eSPI.h>       // Include the graphics library
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <string.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include <TJpg_Decoder.h>
#include <PNGdec.h>
#undef INTELSHORT
#undef INTELLONG
#include <AnimatedGIF.h>
#undef MOTOSHORT
#undef MOTOLONG
#include <JPEGDEC.h>

//#include "pngHelper.h"

#define TJPGD_LOAD_SD_LIBRARY
#define DISPLAY_BUFSIZE 50
#define MENUCOL_BUFSIZE 50
#define MENUROW_BUFSIZE 50
#define BUFFER_SIZE 1024
#define MAX_IMAGE_WIDTH 240
#define UP 33
#define DOWN 25
#define OK   26
#define BACK   27


extern TFT_eSPI tft;  // Create object "tft"


extern char displayBuffer[DISPLAY_BUFSIZE];
extern char menuBuffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE];
extern char secondScreenBuffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE];
extern char textBuffer[BUFFER_SIZE];
extern char selectedPath[MENUCOL_BUFSIZE];

extern int SCREEN_ONE_MENU_ROW;
extern int SCREEN_TWO_MENU_ROW;
extern int SCREEN_THREE_MENU_ROW;

extern unsigned int currentScreen;
extern bool textF;
extern bool jpgF;
extern bool pngF;
extern bool gifF;
extern bool gifPlaying;
extern bool stopgifPlayback;
// Timing
extern unsigned long debounceDelay;
extern unsigned long lastDebounceTime;


//extern int a;
extern int menuArrowYPos;
extern int actualIndex;
extern bool needtoDraw;
extern int rowDraw;
extern int selectedItem;

extern File fileHandle;
extern PNG  png;
extern JPEGDEC jpg;
extern AnimatedGIF gif;
extern uint32_t count;
