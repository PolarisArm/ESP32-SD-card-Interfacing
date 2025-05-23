#include "config.h"


bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
   {
   if (y >= tft.height()) return 0;
   tft.pushImage(x, y, w, h, bitmap);
   return 1;
   }

void jpgDec_init(){
    // Image Scaling
    TJpgDec.setJpgScale(1);
  
    // The byte order can be swapped (set true for TFT_eSPI)
    TJpgDec.setSwapBytes(true);
    
    TJpgDec.setCallback(tft_output);
}
  