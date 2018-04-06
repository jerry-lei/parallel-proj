#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"

//returns the pixel at the given x,y coordinate
struct pixel get_pixel(const struct board* board, const int* x, const int* y)
{
  if((*x)>=board->resolution_x || ((*y)>=board->resolution_y) ||(*x)<0||(*y)<0)
  {
    fprintf(stderr,"ERROR: Trying to access out of bounds. Function get_pixel in image.c.\n");
    return NULL;
  }
  return (board->image)[*x][*y];
}

void set_pixel(const struct board* board, const int* x, const int* y, const int* r, const int* g, const int* b);
{
  if((*x)>=board->resolution_x || ((*y)>=board->resolution_y) ||(*x)<0||(*y)<0)
  {
    fprintf(stderr,"ERROR: Trying to access out of bounds. Function set_pixel in image.c.\n");
  }

  (board->image)[*x][*y].red=*r;
  (board->image)[*x][*y].blue=*b;
  (board->image)[*x][*y].green=*g;
}

//saves the specified board into the file.
void save_ppm(const struct board* board, const char* file){
  FILE* fp;
  int x,y;

  f = fopen(file, 'w');

  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < board->resolution_y; y++ ) {
    for ( x=0; x < board->resolution_x; x++)
      fprintf(f, "%d %d %d ", (board->image)[x][y].red, (board->image)[x][y].green, (board->image)[x][y].blue);
    fprintf(f, "\n");
  }
  fclose(f);
}

*board make_board(const int* res_x, const int* res_y){
  struct board* bred = malloc(sizeof(struct board));
  struct pixel** image = malloc(sizeof(struct pixel**) * res_y)

  if((image[y]=malloc(sizeof(struct pixel*)*res_x))==NULL)
  {
    return NULL;
  }

  for(int y = 0; y < res_y; ++y){
    if((image[y] = malloc(sizeof(pixel)))==NULL)
    {
      return NULL;
    }
  }

  bred->image = image;

  return bred;
}
