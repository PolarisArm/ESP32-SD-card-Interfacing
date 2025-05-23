#include "config.h"



void * myOpen(const char *filename, int32_t *size) {
  fileHandle = SD.open(filename);
  *size = fileHandle.size();
  return &fileHandle;
}
void myClose(void *handle) {
  if (fileHandle) fileHandle.close();
}
int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!fileHandle) return 0;
  return fileHandle.read(buffer, length);
}
int32_t mySeek(JPEGFILE *handle, int32_t position) {
  if (!fileHandle) return 0;
  return fileHandle.seek(position);
}

// Function to draw pixels to the display
int JPEGDraw(JPEGDRAW *pDraw) {
//  Serial.printf("jpeg draw: x,y=%d,%d, cx,cy = %d,%d\n",
//     pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
//     for (int i=0; i<pDraw->iWidth*pDraw->iHeight; i++) {
//      pDraw->pPixels[i] = __builtin_bswap16(pDraw->pPixels[i]);
//     }
    //Serial.printf("JPEG draw: x=%d, y=%d, width=%d, height=%d\n",pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);

    // Use pushImage to display the image chunk
    tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);


  return 1;
}
/*
// Main loop, scan for all .JPG files on the card and display them
void loop() {
  int filecount = 0;
  tft.setCursor(0, 0);
  File dir = SD.open("/");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    if (entry.isDirectory() == false) {
      const char *name = entry.name();
      const int len = strlen(name);
      if (len > 3 && strcasecmp(name + len - 3, "JPG") == 0) {
        Serial.print("File: ");
        Serial.println(name);
        tft.print("File: ");
        tft.println(name);
        if (jpg.open((const char *)name, myOpen, myClose, myRead, mySeek, JPEGDraw)) {
          jpg.decode(0,0,0);
          jpg.close();
        } else {
          Serial.print("error = ");
          Serial.println(jpg.getLastError(), DEC);
        } 
        filecount = filecount + 1;
        if (digitalRead(34) == LOW) {
          // skip delay between images when pushbutton is pressed
          delay(1000);
        }
      }
    }
    entry.close();
  }
  if (filecount == 0) {
    Serial.println("No .JPG files found");
    tft.println("No .JPG files found");
    delay(2000);
  }
}*/