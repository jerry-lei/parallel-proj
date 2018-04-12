#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>


int main(int argc, char* argv[])
{
  struct board* img = load_ppm("thingy.ppm");
  rotate(&img,20.0,255,255,255);
  autocrop_board(&img,255,255,255);
  resize_percent(&img,0.6);
  to_grayscale(&img);
  save_ppm(img, "square_shear.ppm");
  free_board(&img);
  
  return EXIT_SUCCESS;
}
