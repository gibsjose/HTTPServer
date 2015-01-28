#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#define SOCKET_READ_TIMEOUT_SEC 20
#define SOCKET_BUFFER_BYTES 50000
#define MAX_FILE_SIZE_BYTES (50 * 1024 * 1024)

#define _SVID_SOURCE
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <openssl/lhash.h>
#include "uthash/src/uthash.h"
#include "logger.h"


typedef struct key_value
{
    char key[5000];
    char value[5000];
    UT_hash_handle hh;
} key_val_t;

//TODO: Add hash table variable
typedef struct request_data
{
    char type[5000];
    char path[5000];
    char version[5000];
} rqheader_t;

void * requesthandler_run(void * aData_ptr);

//Parse the entire request into request_data structure
void parse_request(rqheader_t * request, char* buffer);

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
char* get_date_header(void);



#endif
