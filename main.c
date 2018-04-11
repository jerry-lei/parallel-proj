#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
//#include <mpi.h>


int main(){

  struct board* img = load_ppm("square.ppm");
  printf("After load ppm\n");
     int a=50;
   int b=50;
   struct pixel test = get_pixel(img,&a,&b);
  printf("R:%d G:%d B:%d\n",test.red,test.green,test.blue);
  shear_x_experiment(&img,20.0);
  shear_y(&img,20.0);
  //shear_x_experiment(&img,10.0);
  shear_x_experiment(&img,20.0);
  int zero = 0;
  resize_board(&img,255,255,255);
  save_ppm(img, "square_shear.ppm");
  free_board(&img);

  return EXIT_SUCCESS;
}
