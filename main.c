#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
//#include <mpi.h>


int main(int argc, char* argv[])
{
  struct board* img = load_ppm("stripes.ppm");
  struct board* img_edit = load_ppm("stripes_tilt.ppm");

  resize_dimension(&img,9,8);
  int gray = to_grayscale(&img);
  resize_dimension(&img_edit,9,8);
  to_grayscale(&img_edit);

  uint64_t hash = hash8_gray(&img,gray);
  uint64_t hash2 = hash8_gray(&img_edit,gray);
  int dist = hamming_distance(&hash,&hash2);
  printf("%d\n", dist);
  
  save_ppm(img, "thingy1.ppm");
  save_ppm(img_edit, "thingy2.ppm");
  
  free_board(&img);
  free_board(&img_edit);

  return EXIT_SUCCESS;
}
