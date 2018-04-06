#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>


int main(){
  struct board* img;
  int dimx=200;
  int dimy=100;
  img = make_board(&dimx,&dimy);
  for(int c1 = 0; c1 < dimx; c1++){
    for(int c2 = 0; c2 < dimy; c2++){
      set_pixel(img, &c1, &c2, 0,0,255);
    }
  }
  save_ppm(img,"bred.ppm");

  free_board(&img);

  return EXIT_SUCCESS;
}
