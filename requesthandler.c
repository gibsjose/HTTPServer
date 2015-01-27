#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  int * lSocketFD = (int *) aData_ptr;
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

  return NULL;
}
