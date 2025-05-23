#include "config.h"



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


void drawWrappedText(const char* text, int16_t x, int16_t y, int16_t maxWidth) {
  Serial.println("Starting text drawing...");
  Serial.printf("Text length: %d\n", strlen(text));
  Serial.printf("Font height: %d\n", tft.fontHeight());
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
  