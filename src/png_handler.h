#include "config.h"

void * pngOpen(const char *filename, int32_t *size) {
    Serial.printf("Attempting to open %s\n", filename);
    fileHandle = SD.open(filename, "r");
    if(!fileHandle){
      Serial.println("Failed to open PNG file");
    }
    *size = fileHandle.size();
    return &fileHandle;
  }
  
  void pngClose(void *handle) {
    File pngfile = *((File*)handle);
    if (pngfile && pngfile.available()) {
      pngfile.close();
    }
  }
  
  int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
    if (!fileHandle) return 0;
    page = page; // Avoid warning
    return fileHandle.read(buffer, length);
  }
  
  int32_t pngSeek(PNGFILE *page, int32_t position) {
    if (!fileHandle) return 0;
    page = page; // Avoid warning
    return fileHandle.seek(position);
  }
  
  void pngDraw(PNGDRAW *pDraw) {
    uint16_t lineBuffer[MAX_IMAGE_WIDTH];
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    tft.pushImage(0, pDraw->y, pDraw->iWidth, 1, lineBuffer);
  }