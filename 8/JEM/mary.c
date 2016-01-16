//include header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


int hand_in_maryiage(int *from_suitor) {//file descriptor of client (read)
  int mary_server;//file descriptor of server end (write)
  char buffer[100];
  
  mkfifo("CHAT", 0644);
  *from_suitor = open("CHAT", O_RDONLY);
  remove("CHAT");
  read(*from_suitor, buffer, sizeof(buffer));  
  //mary_server = open(buffer, O_WRONLY);
  printf("<mary> connection is established: [%s]\n", buffer);
  mary_server = open(buffer, O_WRONLY);

  strncpy(buffer, "Let me take you to PROM!", sizeof(buffer));
  write(mary_server, buffer, sizeof(buffer));

  return mary_server;
}

int main() {  
  int mary_server, from_suitor;
  char buffer[100];
  //mary_server = hand_in_maryiage(&from_suitor);//returns file descriptor of pipe?
  while(1) {
    printf("<mary_server> waiting for connection...\n");
    mary_server = hand_in_maryiage(&from_suitor);//returns file descriptor of pipe?
    
    //while(read(from_suitor, buffer, sizeof(buffer))) {
    //printf("<mary_server> received: [%s]\n", buffer);
  
    printf("<mary> enter message:");
    fgets(buffer, sizeof(buffer), stdin);
    write(mary_server, buffer, sizeof(buffer));
    read(from_suitor, buffer,sizeof(buffer) );
    //strncpy(buffer, "", sizeof(buffer));
    printf("<mary> suitor sent a message: [%s]\n", buffer);


    //write(mary_server, buffer, sizeof(buffer));
    //strncpy(buffer, "", sizeof(buffer));
  }
    close(mary_server);
    // }
  close(from_suitor);
  return 0;
}
