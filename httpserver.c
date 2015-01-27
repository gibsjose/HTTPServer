// Indicates that the thread safe versions of each library should be used.
#define _REENTRANT
#define _BSD_SOURCE

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "requesthandler.h"

#define DEFAULT_SERVER_PORT (8080)
#define DEFAULT_DOCROOT ("./")
#define MAX_CHAR_DOCROOT (1024)
#define MAX_CHAR_LOGFILE (1024)

int gServerSockfd;

// Function declarations.
void parse_args(int aNumArgs, char * aArgs[], unsigned * aPort, char ** aDocRoot,
                char ** aLogFile);
void sigint_handler();

int main(int argc, char * argv[]){
  //Parse the arguments for the port, document root and log file.
  unsigned lPort;
  char * lDocRoot;
  char * lLogFile;
  parse_args(argc, argv, &lPort, &lDocRoot, &lLogFile);

  //TODO: Remove this debugging print block.
  printf("Port: %u\n", lPort);
  printf("Document Root: \"%s\"\n", lDocRoot);
  printf("Log file: \"%s\"\n", lLogFile);

  // Handle the CTRL + C (SIGINT) signal.
  signal(SIGINT, sigint_handler);

  gServerSockfd=socket(AF_INET,SOCK_STREAM,0);

  //Create server/client address structures:
  // Server needs to know both the address of itself and the client
  struct sockaddr_in serveraddr, clientaddr;
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_port = htons(lPort);
  serveraddr.sin_addr.s_addr=INADDR_ANY;

  //Bind: Similar to 'connect()': Associating the socket with the address, without contacting the address
  //Bind, on the server side, tells all incoming data on the server address to go to the specified socket
  int lRetVal_bind = bind(gServerSockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
  if (-1 == lRetVal_bind)
  {
    fprintf(stderr, "bind(): Bad doodoo.\n");
    perror(strerror(errno));
    return errno;
  }

  //Listen:
  //  socket descriptor
  //  size of backlog of connections: Drop connections after N UNHANDLED connections
  int lRetVal_listen = listen(gServerSockfd,10);
  if (-1 == lRetVal_listen)
  {
    fprintf(stderr, "listen(): Bad doodoo.\n");
    perror(strerror(errno));
    return errno;
  }

  int len=sizeof(clientaddr);

  while(1){
    printf("Waiting for connection...\n");
    int clientsocket = accept(gServerSockfd, (struct sockaddr*)&clientaddr, &len);

    if (-1 == clientsocket)
    {
      perror(strerror(errno));
      return errno;
    }

    //If this is a new connection:
    printf("A client connected\n");

    // This is a socket that we need to read from.
    pthread_t lThread;
    if(pthread_create(&lThread, NULL, requesthandler_run, &clientsocket))
    {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }
  }

  // Close the main socket. (This will never get called though because of the
  // infinite while loop above).
  close(gServerSockfd);

  return 0;
}


/*
 * Parse the command line arguments for the relevant arguments.
 */
void parse_args(int aNumArgs, char * aArgs[], unsigned * aPort, char ** aDocRoot,
                char ** aLogFile)
{
  // Initialize with default argument values and overwrite them if they are in
  // the argument list.
  *aPort = DEFAULT_SERVER_PORT;
  *aDocRoot = DEFAULT_DOCROOT;
  *aLogFile = "";

  for(int i = 1; i + 1 < aNumArgs; i = i + 2)
  {
    char * lArgFlag = aArgs[i];
    char * lArgValue = aArgs[i + 1];

    if(!strcmp("-p", lArgFlag))
    {
      *aPort = atoi(lArgValue);
    }
    else if(!strcmp("-docroot", lArgFlag))
    {
      *aDocRoot = lArgValue;
    }
    else if(!strcmp("-logfile", lArgFlag))
    {
      *aLogFile = lArgValue;
    }
  }
}

/*
 * Handle the CTRL + C signal by cleaning up things before terminating.
 */
void sigint_handler()
{
  printf("Got a CTRL + C.  Closing shop...\n");

  // Close the server socket.
  close(gServerSockfd);

  exit(0);
}
