#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <mpi.h>


int main(){
  struct board img;
  img = make_board(1000,500);
  for(int c1 = 0; c1 < 1000; c1++){
    for(int c2 = 0; c2 < 500; c2++){
      set_pixel(&img, &c1, &c2, 0,0,255);
    }
  }

  return EXIT_SUCCESS;
}
