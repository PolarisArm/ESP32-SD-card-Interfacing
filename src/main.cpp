#include "config.h"
#include "png_handler.h"
#include "gif_handler.h"
//#include "jpg_handler.h"
#include "text_handler.h"
#include "file_checker.h"
#include "jpgDechandler.h"

char displayBuffer[DISPLAY_BUFSIZE];
char menuBuffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE];
char secondScreenBuffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE];
char textBuffer[BUFFER_SIZE];
char selectedPath[MENUCOL_BUFSIZE];

int SCREEN_ONE_MENU_ROW = 0;
int SCREEN_TWO_MENU_ROW = 0;
int SCREEN_THREE_MENU_ROW = 0;

const uint8_t maxVisibleMenuItem = 7;
const uint16_t firstItemPos = 20;
const uint16_t menuItemHeight = tft.fontHeight();

unsigned int currentScreen = 1;
bool textF = false;
bool jpgF = false;
bool pngF = false;
bool gifF = false;
bool gifPlaying = false;
bool stopgifPlayback = true;

unsigned long debounceDelay = 150;
unsigned long lastDebounceTime = 0;


//int a = 20;
int menuArrowYPos = 20;
int menuArrowXPos = 20;

int actualIndex=1;
bool needtoDraw = 1;
int rowDraw = 1;
int selectedItem = 1;

TFT_eSPI tft = TFT_eSPI();

File fileHandle;
PNG  png;
JPEGDEC jpg;
AnimatedGIF gif;
uint32_t count = 0;



bool checkIfitisDirectory(fs::FS &fs,const char *dirName, uint8_t level){
  File root = fs.open(dirName);
  if(!root){
    Serial.printf("\nFile not opened %s \n",dirName);
    return false;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    root.close();
    return false;
  } else {
    root.close();
    return true;
  }

}

void listDir(fs::FS &fs, const char *dirName, uint8_t level,
  char buffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE],
  int *MENU_ROW)
{
    Serial.printf("Name: %s \n",dirName);

  if(strcmp(dirName,"/FOUND.000") == 0)
  {
      Serial.printf("Inside cmp: %s \n",dirName);

    return;
  }
  Serial.printf("\nListing directory: %s\n,NOW ROW: %d \n",dirName,MENU_ROW);
  File root = fs.open(dirName);
  size_t pathLen;
  if(!root)
  {
    Serial.println("Fialed to open the directory");
    return;
  }
  if(!root.isDirectory())
  {
    Serial.println("Not a Directory");
    root.close();
    return;
  }

  File file = root.openNextFile();

  while(file)
  {
    if(*MENU_ROW >= MENUROW_BUFSIZE)
    {
      Serial.println("BUFFER IS FULL");
      break;
    }
    if(file.isDirectory()){
      if(strcmp(file.path(), "/FOUND.000") == 0) {
      file = root.openNextFile();
      continue;
      }
      Serial.print("File: ");
      Serial.print(file.name());
      Serial.print(" Path: ");
      Serial.println(file.path());
      pathLen = strlen(file.path());
      
      if(pathLen < MENUCOL_BUFSIZE)
      {
        strcpy(buffer[*MENU_ROW],file.path());
        (*MENU_ROW) += 1;
      } else {
        Serial.println("Path too ling");
        return;
      }
      
      if(level){
        listDir(fs,file.path(),level-1,buffer,MENU_ROW);
      }
    } else {
      Serial.print("File: ");
      Serial.print(file.name());
      Serial.print(" Path: ");
      Serial.println(file.path());
      pathLen = strlen(file.path());
      if(pathLen < MENUCOL_BUFSIZE)
      {
        strcpy(buffer[*MENU_ROW],file.path());
        (*MENU_ROW) += 1;
      } else {
        Serial.println("Path too ling");
        return;
      }
    }
    file = root.openNextFile();

  }
  root.close();
  Serial.printf("NOW ROW: %d \n",*MENU_ROW);

}





int menuXPos = 30; // Would Same of every menu item
int menuYPos = 20; // Will increase every time a new item added. Increment will be the font height.


void setup(void) {
  Serial.begin(115200);
  pinMode(UP,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(OK,INPUT);
  pinMode(BACK,INPUT);

  SPI.setFrequency(80000000);

  if(!SD.begin()){
    Serial.println("SD Card mounting failed");
    return;
  }

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);  
  tft.setFreeFont(FSS12);

  // Init JPGDECODER
 // jpgDec_init();


  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE)
  {
    Serial.println("No SD Card is attached");
    return;
  } else if(cardType == CARD_MMC)  {
    Serial.println("MMC");
  } else if( cardType == CARD_SD){
    Serial.println("SDSC");
  } else if( cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize); 
  listDir(SD,"/", 0,menuBuffer,&SCREEN_ONE_MENU_ROW);
  Serial.println("####################################");

  for(int l = 0; l < SCREEN_ONE_MENU_ROW; l++)
  {
    Serial.println(menuBuffer[l]);
  }

  Serial.println("####################################");

  gif.begin(BIG_ENDIAN_PIXELS);
  tft.fillScreen(TFT_BLACK);

}

int upDownTrue = 0;

void loop()
{
 // delay(10);

   menuYPos = 20;
  // MENU_ROW
  if(needtoDraw && currentScreen == 1)
  {
    actualIndex = 1+rowDraw;
    for(int i = rowDraw; i < (maxVisibleMenuItem + rowDraw); i++)
      {
        if(menuBuffer[i][0] == '\0'){break;}
        snprintf(displayBuffer,sizeof(menuBuffer[i]),"%s",menuBuffer[i]);
        tft.drawString(displayBuffer,menuXPos,menuYPos);
        actualIndex += 1;
        if(actualIndex > SCREEN_ONE_MENU_ROW){ actualIndex = SCREEN_ONE_MENU_ROW;}
        menuYPos += tft.fontHeight();
      }

      tft.drawString(">",menuArrowXPos,menuArrowYPos);
      
      needtoDraw = 0;

      Serial.printf("\nVal k: %d j: %d m: %d",selectedItem,actualIndex,menuYPos);
  }
  else if(needtoDraw && currentScreen == 2 && textF == true){
    tft.fillScreen(TFT_BLACK);

    drawWrappedText(textBuffer,5,tft.fontHeight(),240);
    textF = false;
    selectedPath[0] = '\0';
    needtoDraw = 0;
  }else if(needtoDraw && currentScreen == 2 && jpgF == true){
   // Serial.printf("\nInside Cond: %s\n",selectedPath);
    unsigned long now = millis();
    tft.fillScreen(TFT_SKYBLUE);
    //TJpgDec.drawSdJpg(0,0,selectedPath);
    if (jpg.open(selectedPath, myOpen, myClose, myRead, mySeek, JPEGDraw)) {
      jpg.setPixelType(RGB565_BIG_ENDIAN); // Important
      jpg.decode(0,0,0);
      jpg.close();
    } else {
      Serial.print("error = ");
      Serial.println(jpg.getLastError(), DEC);
    } 
    unsigned long then = millis();
    unsigned long times = then - now;
    Serial.printf("\n Time took: %lu\n",times);

    selectedPath[0] = '\0';
    needtoDraw = 0;
    jpgF = false;
  }else if(needtoDraw && currentScreen == 2 && pngF == true){
    // Serial.printf("\nInside Cond: %s\n",selectedPath);
    int16_t rc = png.open(selectedPath,pngOpen,pngClose,pngRead,pngSeek,pngDraw);
    Serial.println(rc);
    unsigned long now = millis();

    tft.fillScreen(TFT_SKYBLUE);
    if(rc == PNG_SUCCESS){
      Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
      Serial.println("Decoding ");

      rc = png.decode(NULL,0);
      Serial.printf("\nDecode returned: %d\n", rc);
      if (rc != PNG_SUCCESS) {
            Serial.println("PNG decoding failed");
            png.close();
      }      
      unsigned long then = millis();
      unsigned long times = then - now;
      Serial.printf("\n Time took: %lu\n",times);
      Serial.println("PNG decoding Success");

      png.close();
    }      
    selectedPath[0] = '\0';
    needtoDraw = 0;
    pngF = false;
   } else if(needtoDraw && currentScreen == 2 && gifF == true){
    //play_gif(selectedPath);
    if(!gifPlaying){
        tft.fillScreen(TFT_BLACK);

        play_gif_start(selectedPath);
        stopgifPlayback = false;
    }
  }

  if(digitalRead(DOWN) == LOW && currentScreen == 1)
  {
    
    if((millis() - lastDebounceTime >= debounceDelay) && (digitalRead(DOWN) == LOW))
    {
      tft.fillScreen(TFT_BLACK);
      selectedItem++;
      upDownTrue = 0;
      if( selectedItem > rowDraw+maxVisibleMenuItem-1) {rowDraw += 1;}

      if(selectedItem >= actualIndex) {
        selectedItem = 1;
        rowDraw  = 1;
        menuArrowYPos = 20;
      }
      if(rowDraw >= SCREEN_ONE_MENU_ROW){
        Serial.println("ROWDraw == MENU_ROW = 0");
        rowDraw = 0;}

      if(menuArrowYPos > tft.fontHeight() * 6)
      {
        menuArrowYPos = 20 + 6 * tft.fontHeight();
      } else {
      menuArrowYPos = 20 + ((selectedItem-1) *  tft.fontHeight());

      }
       needtoDraw = 1;
       Serial.printf("\n %d %d %d \n",rowDraw,selectedItem,menuArrowYPos);

      lastDebounceTime = 0;
    }
    lastDebounceTime = millis();
  }





  if(digitalRead(OK) == LOW && currentScreen == 1)
  {
    if((millis() - lastDebounceTime >= debounceDelay) && (digitalRead(OK) == LOW))
    {
      if(selectedItem >= 1 && selectedItem <= actualIndex)
      {
        
        Serial.printf("\n %s \n",menuBuffer[selectedItem]);

        if(strlen(menuBuffer[selectedItem]) < MENUCOL_BUFSIZE)
          {
            strcpy(selectedPath, menuBuffer[selectedItem]);
            bool fileCheck = checkIfitisDirectory(SD,selectedPath,1);
              if(fileCheck){
                listDir(SD,selectedPath,1,menuBuffer,&SCREEN_ONE_MENU_ROW);
                selectedPath[0] = '\0';
                needtoDraw = 1;
              } else if(isTextFile(selectedPath)) {
                ReadTextFile(SD,selectedPath,textBuffer);
                textF = true;
                currentScreen = 2;
                needtoDraw = 1;
              } else if(isJpgFile(selectedPath)) {
                jpgF = true;
                currentScreen = 2;
                needtoDraw = 1;
              } else if(isPngFile(selectedPath)) {
                pngF = true;
                currentScreen = 2;
                needtoDraw = 1;
              } else if(isGifFile(selectedPath)) {
                stopgifPlayback = false;
                gifF = true;
                currentScreen = 2;
                needtoDraw = 1;
              }
          
          } else {
          Serial.println("Path too Long");
         }
        
      }
      lastDebounceTime = 0;
    }
    lastDebounceTime = millis();

  }

  if(digitalRead(BACK) == LOW)
  {
    if((millis() - lastDebounceTime >= debounceDelay) && (digitalRead(BACK) == LOW))
    {
      
      if(currentScreen > 1){
        currentScreen = currentScreen - 1;
        if(gifPlaying == true){
          gifPlaying = false;
          stopgifPlayback = true;
        }
        for(int i = 0; i < SCREEN_TWO_MENU_ROW; i++)
        {
          secondScreenBuffer[i][0] = '\0';
        }
        tft.fillScreen(TFT_BLACK);
        SCREEN_TWO_MENU_ROW = 0;

        needtoDraw = 1;
      }
    
      lastDebounceTime = 0;
    }
    lastDebounceTime = millis();
  }
  if(gifPlaying){
      update_gif();

  }
}
