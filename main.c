#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>


int main(){
  struct board* img;
  int dimx=100;
  int dimy=100;
  img = make_board(&dimx,&dimy);

  for(int x = 0; x<dimx;++x)
  {
    int zero = 0;
    set_pixel(img,&x,&zero, x+155,0,0);
  }

  save_ppm(img,"bred.ppm");

  free_board(&img);

  return EXIT_SUCCESS;
}
