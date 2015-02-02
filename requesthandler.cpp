#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  int * lSocketFD = (int *) aData_ptr;
  char line[SOCKET_BUFFER_BYTES];
  rqheader_t r_header;

  // Set a read timeout on the socket.
  struct timeval timeout;
  timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  setsockopt(*lSocketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof(struct timeval));

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
    parse_request(&r_header, line);
    char* response;

    //2. If request type is not implemented (!GET) then build 501 response.
    //   Set the file path to appropriate html response page.
    if(strcmp(r_header.type, "GET")) {
        response = build_501();
    }
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
    free(response);
    close(*lSocketFD);
    pthread_exit(0);

    // Clear the buffer for reading from the client socket.
    memset(&line, 0, SOCKET_BUFFER_BYTES);
  }
}


void parse_request(rqheader_t* rq, char* buffer)
{
    char *line, *line_end = NULL;
    char *token, *tok_end = NULL;

    line = strtok_r(buffer, "\n", &line_end);
    log_info("line: %s\n", line);
    if(line != NULL) {
        token = strtok_r(line, " ", &tok_end);
        memcpy(rq->type, token, strlen(token) + 1);
        token = strtok_r(NULL, " ", &tok_end);
        memcpy(rq->path, token, strlen(token) + 1);
        token = strtok_r(NULL, " ", &tok_end);
        memcpy(rq->version, token, strlen(token) + 1);
    }

    log_info("type: %s\npath: %s\nversion: %s\n", rq->type, rq->path, rq->version);

    while(1) {
        line = strtok_r(NULL, "\n", &line_end);
        if(line == NULL) break;

        //Get key
        char * key = strtok_r(line, ": ", &tok_end);
        if(key == NULL) break;

        //Get value
        char * value = strtok_r(NULL, ": ", &tok_end);
        if(value == NULL) break;

        //Insert the key/value pair into the header map
        rq->header_map.insert(header_pair_t(std::string(key), std::string(value)));
    }

    log_info("Parse succeeded\n");
}

char* build_501(void)
{
    char* response = (char *)malloc(SOCKET_BUFFER_BYTES);
    const char* ver = "version";

    char* str = get_date_header();
    strcat(response, str);
    free(str);

    strcat(response, "Content-Type: text/html\r\n");

    int bytes_read = 0;
    char* file_buffer = (char *)malloc(MAX_FILE_SIZE_BYTES);
    bytes_read = read_file("error_pages/501.html", file_buffer);
    strcat(response, file_buffer);
    strcat(response, "\r\n");

    free(file_buffer);

    return response;
}

//Header format
//Date: Tue, 15 Nov 1994 08:12:31 GMT
//gmtime(), tm struct
char* get_date_header()
{
    char* buffer = (char *)malloc(100 * sizeof(char));
    time_t current_time = time(NULL);
    struct tm * _tm = gmtime(&current_time);
    strftime(buffer, 500, "%a, %d, %b %Y %H:%M:%S %Z\r\n", _tm);
    return buffer;
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
