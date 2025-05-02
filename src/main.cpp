#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>


const char *ssid = "Home";
const char *pass = "353Arm52@89";

void listDir(fs::FS &fs, const char *dirName, uint8_t level)
{
  Serial.printf("Listing directory: %s\n",dirName);

  File root = fs.open(dirName);

  if(!root)
  {
    Serial.println("Failed to open the directory");
    return;
  }
  if(!root.isDirectory())
  {
    Serial.println("Not A directoru");
    return;
  }

  File file = root.openNextFile();
  while ((file))
  {
    if(file){
      if(file.isDirectory())
      {
        Serial.print(" FILE: ");
        Serial.print(file.name());
        if(level){
          listDir(fs,file.path(), level-1);
        }
      }else{
        Serial.print(" FILE: ");
        Serial.print(file.name());
        Serial.print(" SIZE: ");
        Serial.println(file.size());
      }
      file = root.openNextFile();

    }
  }
  
}

void readFile(fs::FS &fs, const char* path)
{
  Serial.printf("Reading FIle: %s\n",path);

  File file = fs.open(path, FILE_READ);

  if(!file){
    Serial.println("Failed to open");
    return;
  }
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
  
}

void writeFile(fs::FS &fs, const char* path, const char* msg)
{
  Serial.printf("Writing FIle: %s\n",path);

  File file = fs.open(path, FILE_APPEND);

  if(!file){
    Serial.println("Failed to open");
    return;
  }
  if(file.print(msg))
  {
    Serial.println("File is written");
  } else {
    Serial.println("Write Failed");
  }
  file.close();
  
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid,pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.println(WiFi.localIP());

  if(!SD.begin())
  {
    Serial.println("SD card mounting failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");

  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);

  readFile(SD, "/hello.txt");


  writeFile(SD, "/hello.txt", "Hello \n\tfrom esp ");
  readFile(SD, "/hello.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  
}

void loop() {

}

