#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "requesthandler.h"

#define DEFAULT_SERVER_PORT 8080

int main(int argc, char * argv[]){
  int sockfd=socket(AF_INET,SOCK_STREAM,0);

  fd_set sockets;
  FD_ZERO(&sockets);

  struct sockaddr_in serveraddr, clientaddr;
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_port = htons(DEFAULT_SERVER_PORT);
  serveraddr.sin_addr.s_addr=INADDR_ANY;

  bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
  listen(sockfd,10);
  FD_SET(sockfd, &sockets);

  int len=sizeof(clientaddr);
  while(1){
    fd_set tmp_set = sockets;
    //max number of selectable sockets, set, ..., ...,
    //timeout for how long select() should wait until at least one is available to read
    select(FD_SETSIZE, &tmp_set, NULL, NULL, NULL); //filters the set
    for(int i = 0; i < FD_SETSIZE; i++)
    {
      //inefficient since it checks ALL sockets rather than just the ones we care about
      if(FD_ISSET(i, &tmp_set))
      {
        //if the file descriptor is still a member of this set
        if(i == sockfd)
        {
          //If this is a new connection:
          printf("A client connected\n");

          int clientsocket = accept(sockfd,
          (struct sockaddr*)&clientaddr,
          &len);

          FD_SET(clientsocket, &sockets);
        }
        else
        {
          // This is a socket that we need to read from.
          pthread_t lThread;
          if(pthread_create(&lThread, NULL, requesthandler_run, &i)) {

            fprintf(stderr, "Error creating thread\n");
            return 1;

          }

          /* wait for the second thread to finish */
          if(pthread_join(lThread, NULL)) {

            fprintf(stderr, "Error joining thread\n");
            return 2;

          }

          // Remove the socket from my set.
          FD_CLR(i, &sockets);
        }
      }
    }
  }

  return 0;
}
