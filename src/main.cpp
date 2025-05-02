#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <string.h>


const char *ssid = "Home";
const char *pass = "353Arm52@89";
const char *mdnsName = "esp";

char *DataInitial;
int32_t *DataValue;

WebServer server(80); // HTTP Server;
WebSocketsServer websocket = WebSocketsServer(82);


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


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{

  switch(type){
    case WStype_CONNECTED:
        {
        IPAddress ip = websocket.remoteIP(num);
        Serial.printf("[%u] Client Connected with bearing ip %d.%d.%d.%d\n",num, ip[0],ip[1],ip[2],ip[3]);
        break;
        }
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Client disconnected",num);
        break;    
    case WStype_TEXT:
        Serial.printf("[%u] Recived text: %s\n",num,payload);
        DataInitial = strtok((char *)payload,":");
        DataValue = (int32_t*)strtok(NULL,":");
        Serial.printf("Data: %s Value: %d",DataInitial,DataValue);
        break;
    case WStype_PING:
        websocket.broadcastPing();
        Serial.println("Recived Ping");
        break;
    case WStype_PONG:
        Serial.printf("[%u] Recived Pong",num);
        break;



  }
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

  if(!MDNS.begin(mdnsName)){
    Serial.println("Error Setting up MDNS Responder");
  } else {
    Serial.println("MDNS Started");
  }
  delay(1000);

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

 /*  listDir(SD, "/", 0);
  readFile(SD, "/Text/LV.txt");
  writeFile(SD, "/Text/LV.txt", "Hello \nfrom esp32 ");


  readFile(SD, "/hello.txt");


  writeFile(SD, "/hello.txt", "Hello \n\tfrom esp ");
  readFile(SD, "/hello.txt");
  writeFile(SD, "/webhello.txt", "Hello \n\tfrom esp ");

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024)); */

  server.on("/",HTTP_GET,[](){
    File file = SD.open("/web/h.html");
    if(!file){
      server.send(500,"text/plain","Failed to open the file");
      return;
    }
    server.streamFile(file,"text/html");
    file.close();
  });

  server.on("/style.css",HTTP_GET,[](){
    File file = SD.open("/web/style.css");
    if(!file){
      server.send(500,"text/plain","Failed to open the css");
      return;
    }
    server.streamFile(file,"text/css");
    file.close();
  });

  server.on("/night.jpg",HTTP_GET,[](){
    File file = SD.open("/web/night.jpg");
    if(!file){
      server.send(500,"text/plain","Failed to open the css");
      return;
    }
    server.streamFile(file,"image/jpeg");
    file.close();
  });

  server.begin();
  websocket.begin();
  websocket.onEvent(webSocketEvent);
  
}

void loop() {
  server.handleClient();
  websocket.loop();
}

