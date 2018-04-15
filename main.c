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
  struct board* original = load_ppm("stop_on_white.ppm");

  to_grayscale(&search);
  to_grayscale(&original);

  int original_dim_x=0;
  int original_dim_y=0;
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