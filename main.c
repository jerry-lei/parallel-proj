#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>


int main(){
  struct board* img;
  int dimx=500;
  int dimy=250;
  img = make_board(&dimx,&dimy);

  for(int x = 0; x<dimx;++x)
  {
    for(int y = 0; y < dimy; ++y){
      set_pixel(img,&x,&y, (x + 100) % 255 , (y),0);
    }
  }

  save_ppm(img,"bred.ppm");

  struct board* copy_board;
  copy_board = load_ppm("bred.ppm");

  save_ppm(copy_board, "dont_change_jerrys_fire_code.ppm");

  free_board(&img);
  free_board(&copy_board);

  return EXIT_SUCCESS;
}
