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
void scale_pixel(struct pixel* pixel, double scale)
{
  pixel->red *= scale;
  pixel->blue *= scale;
  pixel->green *= scale;
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
  int number_ints_read = 0;
  while ((read = getline(&line, &len, fp)) != -1)
  {
    int r, g, b;
    char *ptr = line;
    while (*ptr)
    {
      if (isdigit(*ptr))
      {
        if(number_ints_read >= res_x * 3){
          current_y += 1;
          number_ints_read = 0;
        }
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

int resize_board(struct board** board, int ignore_r, int ignore_g, int ignore_b)
{
  int dim_x = (*board) -> resolution_x;
  int dim_y = (*board) -> resolution_y;
  int left_bound = 0;
  int right_bound = dim_x - 1;
  int up_bound = 0;
  int down_bound = dim_y - 1;
  //get left_bound, right_bound
  for(int c1 = 0; c1 < dim_x; c1++){
    if(right_bound != dim_x - 1 && left_bound != 0) break;
    for(int c2 = 0; c2 < dim_y; c2++){
      if(right_bound != dim_x - 1 && left_bound != 0) break;
      //get the left bound
      struct pixel get_pix = get_pixel(*board, &c1, &c2);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b ){
        left_bound = c1;
      }
      //get the right bound
      int right_c1 = dim_x - c1 - 1;
      int right_c2 = dim_x - c2 - 1;
      get_pix = get_pixel(*board, &right_c1, &right_c2);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b ){
        right_bound = (dim_x-c1-1)+1;
      }
    }
  }
  //get up_bound, down_bound
  for(int c1 = 0; c1 < dim_y; c1++){
    if(down_bound != dim_y - 1 && up_bound != 0) break;
    for(int c2 = 0; c2 < dim_y; c2++){
      if(down_bound != dim_y - 1 && up_bound != 0) break;
      //get the up bound
      struct pixel get_pix = get_pixel(*board, &c1, &c2);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b ){
        up_bound = c1;
      }
      //get the down_bound
      int right_c1 = dim_y - c1 - 1;
      int right_c2 = dim_y - c2 - 1;
      get_pix = get_pixel(*board, &right_c1, &right_c2);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b ){
        down_bound = (dim_y-c1-1)+1;
      }
    }
  }

  int new_dim_x = right_bound - left_bound;
  int new_dim_y = down_bound - up_bound;


  struct board *new_board = make_board(&new_dim_x, &new_dim_y);

  int x_counter = 0;
  int y_counter = 0;
  for(int c1 = 0; c1 < new_dim_x; ++c1){
    y_counter = 0;
    for(int c2 = 0; c2 < new_dim_y; ++c2){
      int start_left = left_bound + c1;
      int start_up = up_bound + c2;
      struct pixel old_pixel = get_pixel(*board, &start_left, &start_up);
      set_pixel(new_board, &x_counter, &y_counter, old_pixel.red, old_pixel.green, old_pixel.blue);
      y_counter++;
    }
    x_counter++;
  }

  free_board(board);
  *board = new_board;
}

int shear_x(struct board **board, double degrees)
{
  //double beta = sin(degrees * (PI / 180.0));
  double alpha = tan((degrees * (PI / 180.0)) / 2);

  int dim_x = abs((alpha) * ((*board)->resolution_x)) + (*board)->resolution_x;
  int dim_y = (*board)->resolution_y;

  //int dim_x=200;
  //int dim_y=200;
  struct board *sheared = make_board(&dim_x, &dim_y);
  if (sheared == NULL)
  {
    return -1;
  }
  for (int y = 0; y < dim_y; ++y)
  {
    for (int x = 0; x < dim_x; ++x)
    {
      set_pixel(sheared, &x, &y, 255, 255, 255);
    }
  }

  int a = 50;
  int b = 50;
  struct pixel test = get_pixel(*board, &a, &b);
  printf("R:%d G:%d B:%d\n", test.red, test.green, test.blue);
  for (int y = 0; y < (*board)->resolution_y; ++y)
  {
    double skew = alpha * y;
    int skewi = floor(skew);
    double skewf = skew - skewi;
    double oleft = 0;
    for (int x = (*board)->resolution_x - 1; x >= 0; --x)
    {
      struct pixel p = get_pixel(*board, &x, &y);
      int left = 0;
      if (x + skewi > 0)
      {
        int new_x = x + skewi;
        set_pixel(sheared, &new_x, &y, p.red, p.green, p.blue);
      }
      oleft = left;
    }
    if (skewi + 1 > 0)
    {
      int new_x = skewi + 1;
      // /set_pixel(sheared,&new_x,&y,oleft,oleft,oleft);
    }
  }
  free_board(board);

  *board = sheared;

  return 0;
}

int shear_x_experiment(struct board **board, double degrees)
{
  //this does antialiasing
  //try on stripes.ppm
  //does not malloc a large enough image when angle increases
  double beta = sin(degrees * (PI / 180.0));

  int dim_x = abs((beta) * ((*board)->resolution_x)) + (*board)->resolution_x;
  int dim_y = (*board)->resolution_y;

  struct board *sheared = make_board(&dim_x, &dim_y);
  if (sheared == NULL)
  {
    return -1;
  }
  for (int y = 0; y < dim_y; ++y)
  {
    for (int x = 0; x < dim_x; ++x)
    {
      set_pixel(sheared, &x, &y, 255, 255, 255);
    }
  }

  //for (int y = 0; y < (*board)->resolution_y; ++y)
  for(int y = (*board)->resolution_y-1; y >= 0; --y)
  {
    double skew = beta * y;

    // /double skew = beta*(y+0.5);
    int skewi = floor(skew);
    double skewf = skew - skewi;
    struct pixel oleft;
    oleft.red=255;
    oleft.green=255;
    oleft.blue=255;
    //for (int x = (*board)->resolution_x-1; x >=0; --x)
    for (int x = 0; x <(*board)->resolution_x; ++x)
    {
      int pos_x = (*board)->resolution_x-x;

      struct pixel pixel = get_pixel(*board, &x, &y);
      struct pixel left;

      left.red=pixel.red;
      left.green=pixel.green;
      left.blue=pixel.blue;
      scale_pixel(&left,skewf);

      pixel.red-=left.red;
      pixel.red+=oleft.red;

      pixel.green-=left.green;
      pixel.green+=oleft.green;

      pixel.blue-=left.blue;
      pixel.blue+=oleft.blue;

      if(x+skewi>0)
      {
        int shear_pos_x = pos_x+skewi;
        set_pixel(sheared,&shear_pos_x,&y,pixel.red,pixel.green,pixel.blue);
      }
      oleft=left;
    }
    if(skewi+1>0)
    {
      int temp = skewi+1;
      set_pixel(sheared,&temp,&y,oleft.red,oleft.green,oleft.blue);
    }
  }
  free_board(board);

  *board = sheared;

  return 0;
}

int shear_y(struct board **board, double degrees)
{
  //double beta = sin(degrees * (PI / 180.0));
  double beta = tan((degrees * (PI / 180.0)) / 2);
  int dim_x = (*board)->resolution_x;
  int dim_y = abs(beta * ((*board)->resolution_y)) + (*board)->resolution_y;

  struct board *sheared = make_board(&dim_x, &dim_y);
  if (sheared == NULL)
  {
    return -1;
  }
  for (int y = 0; y < dim_y; ++y)
  {
    for (int x = 0; x < dim_x; ++x)
    {
      set_pixel(sheared, &x, &y, 255, 255, 255);
    }
  }
  for (int y = 0; y < (*board)->resolution_y; ++y)
  {

    for (int x = 0; x < (*board)->resolution_x; ++x)
    {
      double skew = beta * x;
      int skewi = floor(skew);
      double skewf = skew - skewi;
      double oleft = 0;
      int pos = (*board)->resolution_y - y;
      struct pixel p = get_pixel(*board, &x, &pos);
      int left = 0;
      if (y - skewi > 0)
      {
        int new_y = y - skewi;
        set_pixel(sheared, &x, &new_y, p.red, p.green, p.blue);
      }
      oleft = left;
    }
    //if (skewi + 1 > 0)
    {
      //int new_x = skewi + 1;
      // /set_pixel(sheared,&new_x,&y,oleft,oleft,oleft);
    }
  }
  free_board(board);
  *board = sheared;

  return 0;
}
