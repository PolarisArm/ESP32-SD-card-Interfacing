#include "config.h"

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