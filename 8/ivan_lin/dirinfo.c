#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int read_top_level(char *path, int map_file, int level){
  DIR *dir;

  dir = opendir(path);
  if (dir == NULL){
    printf("Opendir: %s\n",strerror(errno));
  }
  
  struct dirent *dir_file;
  dir_file = readdir(dir);
  
  dup2(map_file, 1);
  
  char name[256];
  char *temp;

  while (dir_file != NULL){
    strcpy(name, path);
    strcat(name, "/");
    strcat(name, dir_file->d_name);
   
    temp = strrchr(name,'/');

    if (strcmp(temp,"/..")!=0 && strcmp(temp,"/.")!=0){
      int i;
      /*for (i=0;i<level;i++){
        printf("  ");
        }*/ 
      if (dir_file->d_type == DT_DIR){
	printf("%s\n",name);
	read_top_level(name, map_file, level+1);
      }else{
	printf("%s\n",name);
      }
    }
    dir_file = readdir(dir);
  }
  closedir(dir);
  return 0;
}

int main( int argc, char *argv[] ){
  int map_file = open("map",O_WRONLY|O_CREAT,0777);
  if (argv[1] == NULL){
    read_top_level(".",map_file,0);
  }else{
    read_top_level(argv[1],map_file,0);
  }
  close(map_file);
}
