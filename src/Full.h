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

//#include "pngHelper.h"

#define TJPGD_LOAD_SD_LIBRARY
#define DISPLAY_BUFSIZE 50
#define MENUCOL_BUFSIZE 50
#define MENUROW_BUFSIZE 50
#define BUFFER_SIZE 1024
#define MAX_IMAGE_WIDTH 240
#define DOWN 14
#define OK   13
#define BACK   16


TFT_eSPI tft = TFT_eSPI();  // Create object "tft"


char displayBuffer[DISPLAY_BUFSIZE];
char menuBuffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE];
char secondScreenBuffer[MENUROW_BUFSIZE][MENUCOL_BUFSIZE];
char textBuffer[BUFFER_SIZE];
char selectedPath[MENUCOL_BUFSIZE];

int SCONE_MENU_ROW = 0;
int SCTWO_MENU_ROW = 0;
int SCTHREE_MENU_ROW = 0;

unsigned int screen = 1;
bool text = false;
bool jpg = false;
bool pngF = false;
bool gifF = false;
bool gifPlaying = false;
bool stopgifPlayback = true;
// Timing
unsigned long debounceDelay = 150;
unsigned long lastDebounceTime = 0;

unsigned long interval = 500;
unsigned long previousMillisdisp = 0;

int a = 20, b = 20;
int j=1;
bool needtoDraw = 1;
int rowDraw = 1;
int OKPoint = 1;

File pngfile;
PNG  png;
AnimatedGIF gif;
static uint32_t count = 0;

void * pngOpen(const char *filename, int32_t *size) {
  Serial.printf("Attempting to open %s\n", filename);
  pngfile = SD.open(filename, "r");
  if(!pngfile){
    Serial.println("Failed to open PNG file");
  }
  *size = pngfile.size();
  return &pngfile;
}

void pngClose(void *handle) {
  File pngfile = *((File*)handle);
  if (pngfile && pngfile.available()) {
    pngfile.close();
  }
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.seek(position);
}



void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  pngfile = SD.open(fname);
  if (pngfile)
  {
    *pSize = pngfile.size();
    return (void *)&pngfile;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > tft.width())
       iWidth = tft.width() - pDraw->iX;
    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    if (y >= tft.height() || pDraw->iX >= tft.width() || iWidth < 1)
       return; 
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }

    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) {
      // Handle transparency
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x = 0, iCount = 0;
      pEnd = pDraw->pPixels + iWidth;
    
      while (x < iWidth) {
        c = ucTransparent - 1;
        int runLength = 0;
        uint16_t usTemp[320]; // temp line buffer
        while (c != ucTransparent && pDraw->pPixels < pEnd && runLength < 320) {
          c = *pDraw->pPixels++;
          if (c != ucTransparent) {
            usTemp[runLength++] = pDraw->pPalette[c];
          } else {
            pDraw->pPixels--; // rewind one
            break;
          }
        }
        if (runLength > 0) {
          tft.pushImage(pDraw->iX + x, pDraw->iY + pDraw->y, runLength, 1, usTemp);
          x += runLength;
        }
    
        // Skip transparent pixels
        runLength = 0;
        while (pDraw->pPixels < pEnd && *pDraw->pPixels == ucTransparent && runLength < 320) {
          runLength++;
          pDraw->pPixels++;
        }
        x += runLength;
      }
    
    } else {
      // No transparency
      uint16_t usTemp[320]; // temp line buffer
      for (int x = 0; x < iWidth; x++) {
        usTemp[x] = pDraw->pPalette[pDraw->pPixels[x]];

      }
      tft.pushImage(pDraw->iX, pDraw->iY + pDraw->y, iWidth, 1, usTemp);
    }
    
}

void play_gif(const char * filename) {

  if (gif.open(filename, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
  {
    while (gif.playFrame(true, NULL))
    {
    }
    gif.close();
  }
}

void update_gif(){
  Serial.printf("\n %d Gif Playing ",gifPlaying);
  if(gifPlaying){
    if(!gif.playFrame(true,NULL) && stopgifPlayback == false){
      gif.reset();
      gif.playFrame(true,NULL);
    } else if(stopgifPlayback == true){
      Serial.println("GIF playback complete");
      gif.close();
      stopgifPlayback = false;
      gifPlaying = false;
      selectedPath[0] = '\0';  // Clear after playing
      needtoDraw = 0;
      gifF = false;
    }
    needtoDraw = 1;

  }
}

void play_gif_start(const char* filename){
  if(gif.open(filename,GIFOpenFile,GIFCloseFile,GIFReadFile,GIFSeekFile,GIFDraw)){
    gifPlaying = true;
  } else {
    gifPlaying = false;
  }
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
   {
   if (y >= tft.height()) return 0;
   tft.pushImage(x, y, w, h, bitmap);
   return 1;
   }


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

bool isTextFile(char *fileName)
{
  if(strstr(fileName,".txt") != NULL){
    return true;
  } 
  return false;
}

bool isJpgFile(char *fileName){
  if(strstr(fileName,".jpg") != NULL){
    return true;
  }
  return false;
}

bool isPngFile(char *fileName){
  if(strstr(fileName,".png") != NULL){
    return true;
  }
  return false;
}

bool isGifFile(char *fileName){
  if(strstr(fileName,".gif") != NULL){
    return true;
  }
  return false;
}

void ReadTextFile(fs::FS &fs,const char* fileName,char* DataBuffer)
{
  File file = fs.open(fileName);
  if(!file){
    Serial.println("Failed to openFile");
    return;
  }
  size_t bytesRead = 0;
  while(file.available()){
    bytesRead = file.readBytes(DataBuffer,BUFFER_SIZE-1);
    DataBuffer[bytesRead] = '\0';
    Serial.print(DataBuffer);
  }
  file.close();
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

int x=30,y = 20;

void drawWrappedText(const char* text, int16_t x, int16_t y, int16_t maxWidth) {
  char line[BUFFER_SIZE] = {0};
  char word[BUFFER_SIZE] = {0};
  int lineLen = 0;
  int wordLen = 0;
  int16_t cursorY = y;
  int16_t lineHeight = tft.fontHeight();

  for (int i = 0; ; i++) {
    char c = text[i];

    if (c == ' ' || c == '\n' || c == '\0') {
      // Add space or newline to word
      word[wordLen] = '\0';

      // Try combining line + word
      char testLine[BUFFER_SIZE];
      strcpy(testLine, line);
      strcat(testLine, word);

      int testWidth = tft.textWidth(testLine);

      if (testWidth > maxWidth && lineLen > 0) {
        // Print current line
        tft.setCursor(x, cursorY);
        tft.print(line);
        cursorY += lineHeight;

        // Move word to new line
        strcpy(line, word);
        lineLen = strlen(line);
      } else {
        strcat(line, word);
        lineLen = strlen(line);
      }

      wordLen = 0;
      word[0] = '\0';

      if (c == '\n') {
        tft.setCursor(x, cursorY);
        tft.print(line);
        cursorY += lineHeight;
        line[0] = '\0';
        lineLen = 0;
      }

      if (c == '\0') {
        if (lineLen > 0) {
          tft.setCursor(x, cursorY);
          tft.print(line);
        }
        break;
      }

      // Add space back if not newline
      if (c == ' ') {
        word[wordLen++] = ' ';
      }

    } else {
      word[wordLen++] = c;
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  pinMode(DOWN,INPUT_PULLDOWN);
  pinMode(OK,INPUT_PULLDOWN);
  pinMode(BACK,INPUT_PULLDOWN);
  SPI.setFrequency(80000000);

  if(!SD.begin()){
    Serial.println("SD Card mounting failed");
    return;
  }
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);  
  tft.setFreeFont(FSS12);

  // Image Scaling
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);
  
  TJpgDec.setCallback(tft_output);


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
  listDir(SD,"/", 0,menuBuffer,&SCONE_MENU_ROW);
  Serial.println("####################################");

  for(int l = 0; l < SCONE_MENU_ROW; l++)
  {
    Serial.println(menuBuffer[l]);
  }

  Serial.println("####################################");
  gif.begin(BIG_ENDIAN_PIXELS);
  tft.fillScreen(TFT_BLACK);

}




void loop()
{
 // delay(10);

   y = 20;
  // MENU_ROW
  if(needtoDraw && screen == 1)
  {
    j = 1+rowDraw;
    for(int i = rowDraw; i < 7+rowDraw; i++)
      {
        if(menuBuffer[i][0] == '\0'){break;}
        snprintf(displayBuffer,sizeof(menuBuffer[i]),"%s",menuBuffer[i]);
        tft.drawString(displayBuffer,x,y);
        j += 1;
        if(j > SCONE_MENU_ROW){ j = SCONE_MENU_ROW;}
        y += tft.fontHeight();
      }
      tft.drawString(">",20,b);
      needtoDraw = 0;

    Serial.printf("\nVal k: %d j: %d",OKPoint,j);
  }
  else if(needtoDraw && screen == 2 && text == true){
    tft.fillScreen(TFT_BLACK);
    drawWrappedText(textBuffer,5,tft.fontHeight(),240);
    text = false;
    selectedPath[0] = '\0';
    needtoDraw = 0;
  }else if(needtoDraw && screen == 2 && jpg == true){
   // Serial.printf("\nInside Cond: %s\n",selectedPath);
    unsigned long now = millis();
    tft.fillScreen(TFT_SKYBLUE);
    TJpgDec.drawSdJpg(0,0,selectedPath);
    unsigned long then = millis();
    unsigned long times = then - now;
    Serial.printf("\n Time took: %lu\n",times);

    selectedPath[0] = '\0';
    needtoDraw = 0;
    jpg = false;
  }else if(needtoDraw && screen == 2 && pngF == true){
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
   } else if(needtoDraw && screen == 2 && gifF == true){
    //play_gif(selectedPath);
    if(!gifPlaying){
        play_gif_start(selectedPath);
        stopgifPlayback = false;
    }
  }

  if(digitalRead(DOWN) == HIGH && screen ==1)
  {
    
    if((millis() - lastDebounceTime >= debounceDelay) && (digitalRead(DOWN) == HIGH))
    {
      tft.fillScreen(TFT_BLACK);
      OKPoint++;
      if( OKPoint > 7) {rowDraw += 1;}

      if(OKPoint >= j) {
        OKPoint = 1;
        rowDraw  = 1;
        b = 20;
      }
      if(rowDraw >= SCONE_MENU_ROW){
        Serial.println("ROWDraw == MENU_ROW = 0");
        rowDraw = 0;}

      if(b > tft.fontHeight() * 6)
      {
        b = 20 + 6 * tft.fontHeight();
      } else {
      b = 20 + ((OKPoint-1) *  tft.fontHeight());

      }
       needtoDraw = 1;
       Serial.printf("\n %d %d %d \n",rowDraw,OKPoint,b);

      lastDebounceTime = 0;
    }
    lastDebounceTime = millis();
  }
  if(digitalRead(OK) == HIGH && screen == 1)
  {
    if((millis() - lastDebounceTime >= debounceDelay) && (digitalRead(OK) == HIGH))
    {
      if(OKPoint >= 1 && OKPoint <= j)
      {
        
        Serial.printf("\n %s \n",menuBuffer[OKPoint]);

        if(strlen(menuBuffer[OKPoint]) < MENUCOL_BUFSIZE)
          {
            strcpy(selectedPath, menuBuffer[OKPoint]);
            bool fileCheck = checkIfitisDirectory(SD,selectedPath,1);
              if(fileCheck){
                listDir(SD,selectedPath,1,menuBuffer,&SCONE_MENU_ROW);
                selectedPath[0] = '\0';
                needtoDraw = 1;
              } else if(isTextFile(selectedPath)) {
                ReadTextFile(SD,selectedPath,textBuffer);
                text = true;
                screen = 2;
                needtoDraw = 1;
              } else if(isJpgFile(selectedPath)) {
                jpg = true;
                screen = 2;
                needtoDraw = 1;
              } else if(isPngFile(selectedPath)) {
                pngF = true;
                screen = 2;
                needtoDraw = 1;
              } else if(isGifFile(selectedPath)) {
                stopgifPlayback = false;
                gifF = true;
                screen = 2;
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

  if(digitalRead(BACK) == HIGH)
  {
    if((millis() - lastDebounceTime >= debounceDelay) && (digitalRead(BACK) == HIGH))
    {
      
      if(screen > 1){
        screen = screen - 1;
        if(gifPlaying == true){
          gifPlaying = false;
          stopgifPlayback = true;
        }
        for(int i = 0; i < SCTWO_MENU_ROW; i++)
        {
          secondScreenBuffer[i][0] = '\0';
        }
        tft.fillScreen(TFT_BLACK);
        SCTWO_MENU_ROW = 0;

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
