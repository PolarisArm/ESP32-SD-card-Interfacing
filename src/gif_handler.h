#include "config.h"


void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  fileHandle = SD.open(fname);
  if (fileHandle)
  {
    *pSize = fileHandle.size();
    return (void *)&fileHandle;
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
  