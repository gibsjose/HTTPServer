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

void * requesthandler_run(void * aData_ptr);
void create_response(char *rBuffer, char** header, int size);
int read_file(char * filepath, char * buffer);
void get_date_header(void);
#endif
