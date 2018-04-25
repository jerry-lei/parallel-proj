#include "image.h"
#include "hash.h"
#include "hsv.h"
#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
//#include <mpi.h>
#include <float.h>


int main(int argc, char* argv[])
{/*
  struct hsv hsv;
  int r,g,b;

  hsv=RGBtoHSV(255,0,255);
  printf("HSV: %f,%f,%f\n",hsv.h,hsv.s,hsv.v);
  HSVtoRGB(hsv,&r,&g,&b);
  printf("RGB: %d,%d,%d\n",r,g,b);
*/
  struct board* search = load_ppm("nick_jerry.ppm");
  struct board* original = load_ppm("wow.ppm");

  struct best_score_info result = find_image(&original,&search,.25);

  bounding_box(&original,&result);

  save_ppm(original,"border.ppm");

  free_board(&search);
  free_board(&original);
  return EXIT_SUCCESS;

}
