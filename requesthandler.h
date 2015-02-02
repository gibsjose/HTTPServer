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
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "logger.h"

typedef struct server_info_t {
    int socket;
    std::string docroot;
}server_info_t;

typedef std::map<std::string, std::string> header_map_t;
typedef std::pair<std::string, std::string> header_pair_t;

typedef struct request_data {
    std::string type;
    std::string path;
    std::string version;
    header_map_t header_map;
} rqheader_t;

void * requesthandler_run(void * aData_ptr);

//Parse the entire request into request_data structure
void parse_request(rqheader_t * request, char* buffer);

//Read appropriate html file for response 501.html
//Append file content to response
//Return pointer to response buffer
std::string build_501(rqheader_t rq);

//Read appropriate html file for response 404.html
//Append file content to response
//Return pointer to response buffer
std::string build_404(rqheader_t rq);

//Read requested file metadata: stat()
//Append file content to response (none for 304)
//Return pointer to response buffer
std::string build_304(rqheader_t rq);

//Read the requested file
//Append file content to response
//Return pointer to response buffer
std::string build_200(rqheader_t request, const std::string &);

//Return content length if read successful
//Otherwise return some negative indicating error
int read_file(std::string filepath, char* buffer);

//TODO: Decide if we are keeping this function
char* get_date_header(void);

//Checks whether a file exists at the given path
bool file_exists(const std::string &);

//Checks to see if the file at 'path' has been modified since the 'time_str'
bool modified_since(const std::string &path, const std::string &time_str);

//Gets the extension of a file
const char *get_filename_ext(const char *filename);

#endif
