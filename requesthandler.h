#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#define SOCKET_READ_TIMEOUT_SEC 20
#define SOCKET_BUFFER_BYTES 50000
#define MAX_FILE_SIZE_BYTES (50 * 1024 * 1024)

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

//TODO: Add hash table variable
struct request_data
{
    char* request_type;
    char* request_path;
    char* verison;
    //hashtable variable
    //Contents
    //date:
    //content-type:
    //etc
};

typedef struct request_data rqheader_t;

void * requesthandler_run(void * aData_ptr);
//Parse the entire request into request_data structure
rqheader_t parse_request(char* buffer);
//Read appropriate html file for response 501.html
//Append file content to response
//Return pointer to response buffer
char* build_501(void);
//Read appropriate html file for response 404.html
//Append file content to response
//Return pointer to response buffer
char* build_404(void);
//Read requested file metadata: stat()
//Append file content to response (none for 304)
//Return pointer to response buffer
char* build_304(void);
//Read the requested file
//Append file content to response
//Return pointer to response buffer
char* build_200(rqheader_t* request);
//Return content length if read successful
//Otherwise return some negative indicating error
int read_file(char * filepath, char* buffer);

//TODO: Decide if we are keeping this function
void get_date_header(void);



#endif
