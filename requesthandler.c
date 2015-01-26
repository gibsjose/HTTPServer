#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  int * lSocketFD = (int *) aData_ptr;
  char line[5000];
  char *firstline;
  char s[2] = "\n";
  recv(*lSocketFD,line,5000,0);

  //We really only care about the first line so we get the token of just the
  //first line
  firstline = strtok(line, s);
  send(*lSocketFD,line,strlen(firstline),0);
  printf("Got from client: %s\n", firstline);
  char *list[10];
  char *p;
  int number = 0;
  char *tokens;
  p = strtok(firstline, " ");
  while(p != NULL) {
    list[number++] = p;
    p = strtok(NULL, " ");
  }
  for (int i = 0; i < 10; i++) {
    if(list[i] == NULL) {
      break;
    }
    printf("%s\n", list[i]);
  }

  //error checking
  printf("Why not");
  if(strcmp(list[1], "GET") == 0) {
    send(*lSocketFD,"501",3,0);
  }
  else if(strcmp(list[2], "-docroot") != 0) {
    printf("cool");
    printf("%s", list[2]);
    char* contents_chopped = list[2] + 1;
    if(access(contents_chopped, F_OK) != -1) {
      printf("cool 2");
      send(*lSocketFD,"200",3,0);
    }
  }
  //need something for fname
  // else if(access(fname, F_OK ) == -1 ) {
  //   send(*lSocketFD,"404",3,0);
  // }

  close(*lSocketFD);

  return NULL;
}
