#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include "image.h"
#include "boolean.h"

//returns the pixel at the given x,y coordinate
struct pixel get_pixel(const struct board *board, const int *x, const int *y)
{
  if ((*x) >= board->resolution_x || ((*y) >= board->resolution_y) || (*x) < 0 || (*y) < 0)
  {
    #ifndef NO_ERROR
    fprintf(stderr, "ERROR: Trying to get out of bounds\n");
    #endif
  }
  return (board->image)[*y][*x];
}

void set_pixel(const struct board *board, const int *x, const int *y, int r, int g, int b)
{
  if ((*x) >= board->resolution_x || ((*y) >= board->resolution_y) || (*x) < 0 || (*y) < 0)
  {
    #ifndef NO_ERROR
    fprintf(stderr, "ERROR: Trying to set out of bounds\n");
    fprintf(stderr, "x:%d y:%d\n", *x, *y);
    #endif
  }

  //add out of bounds color checking

  (board->image)[*y][*x].red = r;
  (board->image)[*y][*x].blue = b;
  (board->image)[*y][*x].green = g;
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
      fprintf(fp, "%d %d %d ", (board->image)[y][x].red, (board->image)[y][x].green, (board->image)[y][x].blue);
    fprintf(fp, "\n");
  }
  fclose(fp);
}

struct board *make_board(const int *res_x, const int *res_y)
{
  struct board *bred = malloc(sizeof(struct board));
  struct pixel **image;

  if ((image = malloc(sizeof(struct pixel *) * (*res_y))) == NULL)
  {
    return NULL;
  }

  for (int y = 0; y < (*res_y); ++y)
  {
    if ((image[y] = malloc(sizeof(struct pixel) * (*res_x))) == NULL)
    {
      return NULL;
    }
  }

  bred->image = image;
  bred->resolution_x = (*res_x);
  bred->resolution_y = (*res_y);

  return bred;
}

struct board *load_ppm(const char *file)
{
  FILE *fp;
  int res_x, res_y;

  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(file, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "ERROR: Could not open file\n");
    return NULL;
  }
  //read 3 lines before making a board
  int number_lines_read = 0;
  while ((read = getline(&line, &len, fp)) != -1)
  {
    //comment lines
    if (line[0] == '#')
    {
      continue;
    }
    //descriptor lines (p3, dimensions, max_color)
    else
    {
      if (number_lines_read == 0)
      {
        if (line[0] != 'P')
        {
          fprintf(stderr, "ERROR: Bad image descriptor\n");
          return NULL;
        }
      }
      else if (number_lines_read == 1)
      {
        int number_ints_read = 0;
        char *ptr = line;
        while (*ptr)
        {
          if (isdigit(*ptr))
          {
            long val = strtol(ptr, &ptr, 10);
            //this is so bad i don't even know what i'm thinking
            if (number_ints_read == 0)
            {
              res_x = val;
            }
            else
            {
              res_y = val;
            }
            number_ints_read += 1;
          }
          else
          { // Otherwise, move on to the next character.
            ptr++;
          }
        }
        if (number_ints_read != 2)
        {
          fprintf(stderr, "ERROR: Corrupted file\n");
          return NULL;
        }
      }
      else if (number_lines_read == 2)
      {
        //this number isn't important.
        break;
      }
      number_lines_read++;
    }
  }

  struct board *new_board;
  new_board = make_board(&res_x, &res_y);
  //fire we made the board
  //read in the pixels
  int current_y = 0;
  while ((read = getline(&line, &len, fp)) != -1)
  {
    int r, g, b;
    int number_ints_read = 0;
    char *ptr = line;
    while (*ptr)
    {
      if (isdigit(*ptr))
      {
        long val = strtol(ptr, &ptr, 10);
        //this is so bad i don't even know what i'm thinking
        if (number_ints_read % 3 == 0)
        {
          r = val;
        }
        else if (number_ints_read % 3 == 1)
        {
          g = val;
        }
        else if (number_ints_read % 3 == 2)
        {
          b = val;
          int current_x = number_ints_read / 3;
          set_pixel(new_board, &current_x, &current_y, r, g, b);
        }
        number_ints_read += 1;
      }
      else
      { // Otherwise, move on to the next character.
        ptr++;
      }
    }
    current_y += 1;
  }
  fclose(fp);
  if (line)
    free(line);
  return new_board;
}

void free_board(struct board **board)
{
  for (int y = 0; y < (*board)->resolution_y; ++y)
  {
    free(((*board)->image)[y]);
  }
  free((*board)->image);
  free(*board);
}

int sheer_x(struct board **board, double degrees)
{
  double beta = sin(degrees * (PI / 180.0));
  double alpha = tan((degrees * (PI / 180.0)) / 2);

  int dim_x = abs((2 * alpha) * ((*board)->resolution_x))+(*board)->resolution_x;
  int dim_y = (*board)->resolution_y;
  printf("x:%d  y:%d\n",dim_x,dim_y);
  struct board *sheered = make_board(&dim_x, &dim_y);


  for (int y = 0; y < (*board)->resolution_y; ++y)
  {
    double skew = alpha * (y + 0.5);
    double skew_i = floor(skew);
    double skew_f = skew - skew_i;
    double oleft = 0.0;

    for (int x = 0; x < (*board)->resolution_x; ++x)
    {
      int x_p = x - beta * y;
      int x_pos = (*board)->resolution_x - x;
      struct pixel pix = get_pixel(*board, &x, &y);

      set_pixel(sheered, &x_p, &y, pix.red, pix.green, pix.blue);
    }
  }
save_ppm(sheered,"sheer_wood.ppm");
  free_board(board);

  board = &sheered;

  return TRUE;
}