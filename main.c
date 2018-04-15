#include "image.h"
#include "hash.h"

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

  resize_dimension(&search,275,275);

  /*int original_dim_x=-1;
  int original_dim_y=-1;
  struct color_hash** hashed_original_avg = hash_avg_original_color(&original,&original_dim_x,&original_dim_y);
  split_avg_hash_color(&search,hashed_original_avg, original_dim_x, original_dim_y);*/

  int original_dim_x=-1;
  int original_dim_y=-1;
  struct color_hash** hashed_original = hash_original_color(&original,&original_dim_x,&original_dim_y);
  split_hash_color(&search,hashed_original, original_dim_x, original_dim_y);

  to_grayscale(&search);
  to_grayscale(&original);
  original_dim_x=-1;
  original_dim_y=-1;
  uint64_t** hashed_original2 = hash_original_gray(&original,&original_dim_x,&original_dim_y);
  split_hash_gray(&search,hashed_original2, original_dim_x, original_dim_y);

  free_board(&search);
  free_board(&original);
  for(int y = 0; y < original_dim_y; ++y)
  {
   free(hashed_original[y]);
  }
  free(hashed_original);
  return EXIT_SUCCESS;
}
