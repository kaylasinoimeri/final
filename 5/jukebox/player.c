#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef LINUX

int playsong(char * song){
  char * pointers[3];
  
  #ifdef LINUX
  pointers[0] = "mpg321";
  #else
  pointers[0] = "afplay";
  #endif
  pointers[1] = song;
  pointers[2] = NULL;
  execvp(pointers[0], pointers);
  return 0;
}
