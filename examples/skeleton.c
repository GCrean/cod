// skeleton.c - This is a skeleton application that doesn't do anything except exit
#include <stdio.h>
#include <stdlib.h>

#include "cod.h"

int main(void) {
  int running = 1;

  if(!cod_open(640, 480)) {
    printf("%s\n", cod_get_error());
    return EXIT_FAILURE;
  }

  cod_set_title("skeleton");

  cod_swap();

  cod_event e;
  
  while(running) {
    while(cod_get_event(&e)) {
      switch(e.type) {
      case COD_QUIT:
	running = 0;
	break;
      }
    }

    cod_sleep(10000);

    cod_swap();
  }
  
  cod_close();
  return EXIT_SUCCESS;
}
