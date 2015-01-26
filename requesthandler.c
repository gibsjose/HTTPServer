#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  int * lSocketFD = (int *) aData_ptr;
  char line[SOCKET_BUFFER_BYTES];

  // Set a read timeout on the socket.
  struct timeval timeout;
  timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  setsockopt(*lSocketFD, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

  while(1)
  {
    printf("Receiving...\n");
    size_t lRetVal_recv = recv(*lSocketFD, line, SOCKET_BUFFER_BYTES, 0);
    if (0 == lRetVal_recv)
    {
      // Client likely closed the connection.
      pthread_exit(NULL);
    }
    else if(-1 == lRetVal_recv)
    {
      //some other error
      fprintf(stderr, "recv(): Bad doodoo.\n");
      perror(strerror(errno));
      pthread_exit(NULL);
    }
    else
    {
      printf("Received %lu bytes.\n", lRetVal_recv);
    }

    printf("Got from client:\n");
    printf("-----------------------------\n");
    printf("%s\n", line);
    printf("-----------------------------\n");

    // TODO: Change me.  I just send back the same data I was sent.
    printf("Sending...\n");
    size_t lRetVal_send = send(*lSocketFD,line,strlen(line),0);
    if(-1 == lRetVal_send)
    {
      fprintf(stderr, "send(): Bad doodoo.\n");
      perror(strerror(errno));
      pthread_exit(NULL);
    }
    else
    {
      printf("Sent %lu bytes.\n", lRetVal_send);
    }

    /*
     * In HTTP 1.1 all connections are considered persistent unless declared otherwise.
     * http://en.wikipedia.org/wiki/HTTP_persistent_connection
     * So close the socket if the Connection: close header is sent.
     */

    // TODO: Change me to intelligently close and exit.
    close(*lSocketFD);
    pthread_exit(0);

    // Clear the buffer for reading from the client socket.
    memset(&line, 0, SOCKET_BUFFER_BYTES);
  }
}
