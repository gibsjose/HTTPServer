#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  int * lSocketFD = (int *) aData_ptr;
  char line[5000];
  recv(*lSocketFD,line,5000,0);
  printf("Got from client: %s\n",line);
  send(*lSocketFD,line,strlen(line),0);
  close(*lSocketFD);

  return NULL;
}
