#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"

//returns the pixel at the given x,y coordinate
struct pixel get_pixel(const struct board *board, const int *x, const int *y)
{
  if ((*x) >= board->resolution_x || ((*y) >= board->resolution_y) || (*x) < 0 || (*y) < 0)
  {
    fprintf(stderr, "ERROR: Trying to access out of bounds. Function get_pixel in image.c.\n");
  }
  return (board->image)[*x][*y];
}

void set_pixel(const struct board *board, const int *x, const int *y, int r, int g, int b)
{
  if ((*x) >= board->resolution_x || ((*y) >= board->resolution_y) || (*x) < 0 || (*y) < 0)
  {
    fprintf(stderr, "ERROR: Trying to access out of bounds. Function set_pixel in image.c.\n");
    fprintf(stderr, "x:%d y:%d\n", *x, *y);
  }

  //add out of bounds color checking

  (board->image)[*x][*y].red = r;
  (board->image)[*x][*y].blue = b;
  (board->image)[*x][*y].green = g;
}

//saves the specified board into the file.
void save_ppm(const struct board *board, const char *file)
{
  FILE *fp;
  int x, y;

  fp = fopen(file, "w");

  fprintf(fp, "P3\n%d %d\n%d\n", board->resolution_x, board->resolution_y, MAX_COLOR);
  for (y = 0; y < board->resolution_y; y++)
  {
    for (x = 0; x < board->resolution_x; x++)
      fprintf(fp, "%d %d %d ", (board->image)[x][y].red, (board->image)[x][y].green, (board->image)[x][y].blue);
    fprintf(fp, "\n");
  }
  fclose(fp);
}

struct board *make_board(const int *res_x, const int *res_y)
{
  struct board *bred = malloc(sizeof(struct board));
  struct pixel **image;

  if ((image = malloc(sizeof(struct pixel *) * (*res_x))) == NULL)
  {
    return NULL;
  }

  for (int x = 0; x < (*res_x); ++x)
  {
    if ((image[x] = malloc(sizeof(struct pixel) * (*res_y))) == NULL)
    {
      return NULL;
    }
  }

  bred->image = image;
  bred->resolution_x = (*res_x);
  bred->resolution_y = (*res_y);

  return bred;
}

void free_board(struct board **board)
{
  for (int x = 0; x < (*board)->resolution_x; ++x)
  {
    free(((*board)->image)[x]);
  }
  free((*board)->image);
  free(*board);
}