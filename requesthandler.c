#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  int * lSocketFD = (int *) aData_ptr;
<<<<<<< HEAD
  char line[5000];
  char *firstline;
  char s[2] = "\n";
  recv(*lSocketFD,line,5000,0);
  send(*lSocketFD,line,strlen(line),0);
  printf("Got from client: %s\n", line);

  firstline = strtok(line ,s);
  int temp = 0;
  char *p;
  char *array[3];
  p = strtok(firstline, " ");
  while(p != NULL) {
    array[temp++] = p;
    p = strtok(NULL, " ");
  }
  array[1] = array[1] + 1;

  //starting the error checking
  //Sending 501 status if anything but GET
  if(strcmp(array[0], "GET") != 0) {
    char *topage;
    topage = "\n\n501";
    send(*lSocketFD,topage,strlen(topage),0);
  }
  //Checks to see if file exists in default directory
  //Will implement stat (man 2 stat) to use different directory
  if( access( array[1], F_OK ) != -1 ) {
    char *topage;
    topage = "\n\n200";
    send(*lSocketFD,topage,strlen(topage),0);
  }

  close(*lSocketFD);
=======
  char line[SOCKET_BUFFER_BYTES];

  // Set a read timeout on the socket.
  struct timeval timeout;
  timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  setsockopt(*lSocketFD, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
>>>>>>> FETCH_HEAD

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

    //Put together a response header
    //Response header example
    // HTTP/1.1 <status_code> <status_label>
    // HTTP/1.1 200 OK
    // Date: Fri, 31 Dec 1999 23:59:59 GMT
    // Content-Type: text/html
    // Content-Length: 1354

    //1. Parse the entire request header
    // Build up a data stucture describing request
    // use parse_request function

    //2. If request type is not implemented (!GET) then build 501 response.
    //   Set the file path to appropriate html response page.

    //3. Check if the file exists. If not send 404 response.
    //   Set the file path to appropriate html response page.

    //4. Check for if-modified since header in the request. If found and the requested
    //   file has not been modified, send a 304 response.
    //   Set the file path to appropriate html response page. (probably no page for this so filepath null)

    //5. Create status 200 reponse.
    //   Set the file path to appropriate html response page (requested page)

    //6. Read the appropriate file (filepath variable set by the responses)


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

//Header format
//Date: Tue, 15 Nov 1994 08:12:31 GMT
//gmtime(), tm struct
void get_date_header()
{
    //time_t cur_time = time(NULL);
    //struct tm tm = gmtime(&cur_time);
}

int read_file(char * filepath, char * lBuffer)
{
    size_t lTotalBytesRead = 0;
    FILE * lFilePtr = fopen(filepath, "rb");

    if(lFilePtr == 0) {
        perror(strerror(errno));
        return -1;
    }
    while (!feof(lFilePtr)) {
        int lNumBytesRead = fread(lBuffer, 1, MAX_FILE_SIZE_BYTES, lFilePtr);
        lTotalBytesRead += lNumBytesRead;
        if(lNumBytesRead == 0) {
            if (ferror(lFilePtr)) {
                printf("There was an error reading the file.\n");
                return -1;
            }
            else {
                //done reading the file
                return lTotalBytesRead;
            }
        }
    }
}
