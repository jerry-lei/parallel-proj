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
  struct board* search = load_ppm("stop.ppm");
  struct board* original = load_ppm("landscape.ppm");

  //resize_dimension(&search,50,50);

  to_grayscale(&search);
  autocrop_board(&search,255,255,255);
  to_grayscale(&original);
  save_ppm(search,"grey.ppm");

  int original_dim_x=-1;
  int original_dim_y=-1;
  uint64_t** hashed_original = hash_original(&original,&original_dim_x,&original_dim_y);
  split_hash(&search,hashed_original, original_dim_x, original_dim_y);

  free_board(&search);
  free_board(&original);
  for(int y = 0; y < original_dim_y; ++y)
  {
   free(hashed_original[y]);
  }
  free(hashed_original);
  return EXIT_SUCCESS;
}
