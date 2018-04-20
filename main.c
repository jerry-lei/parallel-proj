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


int main(int argc, char* argv[])
{
  struct board* search = load_ppm("nick_jerry.ppm");
  struct board* original = load_ppm("wow.ppm");

  resize_dimension(&search,200,200);
  //autocrop_board(&search,255,255,255);

  int original_dim_x=-1;
  int original_dim_y=-1;
  struct hsv_hash** hashed_original = hash_original_HSV(&original,&original_dim_x,&original_dim_y);
  split_hash_HSV(&search,hashed_original, original_dim_x, original_dim_y);

  free_board(&search);
  free_board(&original);
  for(int y = 0; y < original_dim_y; ++y)
  {
   free(hashed_original[y]);
  }
  free(hashed_original);
  return EXIT_SUCCESS;
}
