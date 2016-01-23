#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main() {

  int socket_id, socket_client;
  int exit_status = 0;
  char input[1024];
  
  //create the socket
  socket_id = socket( AF_INET, SOCK_STREAM, 0 );
  
  //bind to port/address
  struct sockaddr_in listener;
  listener.sin_family = AF_INET;  //socket type IPv4
  listener.sin_port = htons(6001); //port #
  listener.sin_addr.s_addr = INADDR_ANY; //bind to any incoming address
  bind(socket_id, (struct sockaddr *)&listener, sizeof(listener));
  
  listen( socket_id, 1 );
  printf("<server> listening\n");

  socket_client = accept( socket_id, NULL, NULL );
  printf("<server> connected: %d\n", socket_client );

  //fgets something
  printf("enter a message for the client: ");
  fgets(input, sizeof(input), stdin);
  write( socket_client, input, sizeof(input));
  exit_status ++;
  
  exit_status = 0;

  return 0;
}
