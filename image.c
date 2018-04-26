#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include "image.h"
#include "boolean.h"
#include "hash.h"
#include "hsv.h"
#include "score.h"


//returns the pixel at the given x,y coordinate
struct pixel get_pixel(const struct board *board, const int *x, const int *y)
{
  if ((*x) >= board->resolution_x || ((*y) >= board->resolution_y) || (*x) < 0 || (*y) < 0)
  {
#ifndef NO_ERROR
    fprintf(stderr, "ERROR: Trying to get out of bounds\n");
    fprintf(stderr, "x:%d y:%d\n", *x, *y);
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
  if (r <= 255)
  {
    (board->image)[*y][*x].red = r;
  }
  else
  {
    (board->image)[*y][*x].red = r;
  }

  if (r <= 255)
  {
    (board->image)[*y][*x].blue = b;
  }
  else
  {
    (board->image)[*y][*x].blue = b;
  }

  if (r <= 255)
  {
    (board->image)[*y][*x].green = g;
  }
  else
  {
    (board->image)[*y][*x].green = g;
  }
}
void scale_pixel(struct pixel *pixel, double scale)
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

  if ((image = calloc((*res_y), sizeof(struct pixel *) )) == NULL)
  {
    return NULL;
  }

  for (int y = 0; y < (*res_y); ++y)
  {
    if ((image[y] = calloc((*res_x),sizeof(struct pixel)))  == NULL)
    {
      return NULL;
    }
  }

  bred->image = image;
  bred->resolution_x = (*res_x);
  bred->resolution_y = (*res_y);

  return bred;
}

struct board *copy_board(const struct board* old_board){
  int old_x = old_board -> resolution_x;
  int old_y = old_board -> resolution_y;
  struct board* new_board = make_board(&old_x, &old_y);
  for(int row = 0; row < old_y; ++row){
    for(int col = 0; col < old_x; ++col){
      struct pixel old_pixel = get_pixel(old_board, &col, &row);
      set_pixel(new_board, &col, &row, old_pixel.red, old_pixel.green, old_pixel.blue);
    }
  }
  return new_board;
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
        if (number_ints_read >= res_x * 3)
        {
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

int autocrop_board(struct board** board, int ignore_r, int ignore_g, int ignore_b)
{
  int dim_x = (*board) -> resolution_x;
  int dim_y = (*board) -> resolution_y;
  int left_bound = 0;
  int right_bound = dim_x - 1;
  int up_bound = 0;
  int down_bound = dim_y - 1;
  //get left_bound, right_bound
  for(int c1 = 0; c1 < dim_x; c1++){
    if(left_bound != 0) break;
    for(int c2 = 0; c2 < dim_y; c2++){
      if(left_bound != 0) break;
      //get the left bound
      struct pixel get_pix = get_pixel(*board, &c1, &c2);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b )
        left_bound = c1;
    }
  }
  for(int c1 = dim_x - 1; c1 >= 0; c1--){
    if(right_bound != dim_x-1) break;
    for(int c2 = dim_y - 1; c2 >= 0; c2--){
      if(right_bound != dim_x-1) break;
      //get the left bound
      struct pixel get_pix = get_pixel(*board, &c1, &c2);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b )
        right_bound = c1;
    }
  }
  //get up_bound, down_bound
  for(int c1 = 0; c1 < dim_y; c1++){
    if(up_bound != 0) break;
    for(int c2 = 0; c2 < dim_x; c2++){
      if(up_bound != 0) break;
      //get the up bound
      struct pixel get_pix = get_pixel(*board, &c2, &c1);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b )
        up_bound = c1;
    }
  }
  for(int c1 = dim_y - 1; c1 >= 0; c1--){
    if(down_bound != dim_y-1) break;
    for(int c2 = dim_x - 1; c2 >= 0; c2--){
      if(down_bound != dim_y-1) break;
      //get the up bound
      struct pixel get_pix = get_pixel(*board, &c2, &c1);
      if(get_pix.red != ignore_r || get_pix.green != ignore_g || get_pix.blue != ignore_b )
        down_bound = c1;
    }
  }

  int new_dim_x = abs(right_bound - left_bound-1);
  int new_dim_y = abs(down_bound - up_bound-1);

  if(new_dim_x == dim_x && new_dim_y == dim_y){
    return 0;
  }

  struct board *new_board = make_board(&new_dim_x, &new_dim_y);

  for(int c1 = 0; c1 < new_dim_x; ++c1){
    for(int c2 = 0; c2 < new_dim_y; ++c2){
      int start_left = left_bound + c1 + 1;
      int start_up = up_bound + c2 + 1;
      struct pixel old_pixel = get_pixel(*board, &start_left, &start_up);
      set_pixel(new_board, &c1, &c2, old_pixel.red, old_pixel.green, old_pixel.blue);
    }
  }

  free_board(board);
  *board = new_board;


  return 0;
}

int shear_x(struct board **board, double degrees, int ignore_r, int ignore_g, int ignore_b)
{
  double beta = tan((degrees * (PI / 180.0)) / 2);

  int dim_x = ceil(fabs(beta) * (*board)->resolution_y) + (*board)->resolution_x;
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

  int max_y = -1;
  int min_y = dim_y + 1;
  int max_x = -1;
  int min_x = dim_x + 1;
  for (int y = 0; y < (*board)->resolution_y; ++y)
  {
    double skew = beta * y;
    int skewi = floor(skew);
    double skewf = skew - skewi;
    struct pixel oleft;
    oleft.red = 255;
    oleft.green = 255;
    oleft.blue = 255;
    for (int x = 0; x < (*board)->resolution_x; ++x)
    {

      struct pixel pixel = get_pixel(*board, &x, &y);
      struct pixel left;

      left.red = pixel.red;
      left.green = pixel.green;
      left.blue = pixel.blue;
      scale_pixel(&left, skewf);

      pixel.red -= left.red;
      pixel.red += oleft.red;

      pixel.green -= left.green;
      pixel.green += oleft.green;

      pixel.blue -= left.blue;
      pixel.blue += oleft.blue;
      if(degrees > 0.0){
        if (x + skewi > 0)
        {
          int new_x = x + skewi;
          if(pixel.red > 255) pixel.red = 255;
          if(pixel.green > 255) pixel.green = 255;
          if(pixel.blue > 255) pixel.blue = 255;
          set_pixel(sheared, &new_x, &y, pixel.red, pixel.green, pixel.blue);
          if(pixel.red != ignore_r || pixel.green != ignore_g || pixel.blue != ignore_b){
            if(y < min_y) min_y = y;
            if(y > max_y) max_y = y;
            if(new_x < min_x) min_x = new_x;
            if(new_x > max_x) max_x = new_x;
          }
        }
        oleft = left;
      }
      else{
        if ((x + skewi + ((dim_x) - (*board) -> resolution_x)) > 0){
          int new_x = x + skewi + (dim_x) - (*board) -> resolution_x;
          if(pixel.red > 255) pixel.red = 255;
          if(pixel.green > 255) pixel.green = 255;
          if(pixel.blue > 255) pixel.blue = 255;
          set_pixel(sheared, &new_x, &y, pixel.red, pixel.green, pixel.blue);
          if(pixel.red != ignore_r || pixel.green != ignore_g || pixel.blue != ignore_b){
            if(y < min_y) min_y = y;
            if(y > max_y) max_y = y;
            if(new_x < min_x) min_x = new_x;
            if(new_x > max_x) max_x = new_x;
          }
        }
        oleft = left;
      }
    }
  }

  int smaller_y = max_y - min_y+1;
  int smaller_x = max_x - min_x+1;
  struct board* resized_board = make_board(&smaller_x,&smaller_y);
  for(int c1 = 0; c1 < smaller_x; c1++){
    for(int c2 = 0; c2 < smaller_y; c2++){
      int altered_x = min_x + c1;
      int altered_y = min_y + c2;
      struct pixel old = get_pixel(sheared, &altered_x, &altered_y);
      set_pixel(resized_board,&c1,&c2, old.red, old.green, old.blue);
    }
  }
  free_board(board);
  free_board(&sheared);
  *board = resized_board;

  return 0;
}


int shear_y(struct board **board, double degrees, int ignore_r, int ignore_g, int ignore_b)
{
  double alpha = sin((0-degrees) * (PI / 180.0));
  //double alpha = 0.34;
  //double alpha = tan((degrees * (PI / 180.0)) / 2);
  int dim_x = (*board)->resolution_x;
  int dim_y = ceil(fabs(alpha) * (*board)->resolution_x) + (*board)->resolution_y;

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
  int max_y = -1;
  int min_y = dim_y + 1;
  int max_x = -1;
  int min_x = dim_x + 1;
  for (int x = 0; x < (*board)->resolution_x; ++x)
  {

    double skew = alpha * x;
    int skewi = floor(skew);
    double skewf = skew - skewi;
    struct pixel oleft;
    oleft.red = 255;
    oleft.green = 255;
    oleft.blue = 255;

    for (int y = 0; y < (*board)->resolution_y; ++y)
    {
      int pos = (*board)->resolution_y - y-1;
      struct pixel pixel = get_pixel(*board, &x, &y);
      struct pixel left;

      left.red = pixel.red;
      left.green = pixel.green;
      left.blue = pixel.blue;
      scale_pixel(&left, skewf);

      pixel.red -= left.red;
      pixel.red += oleft.red;

      pixel.green -= left.green;
      pixel.green += oleft.green;

      pixel.blue -= left.blue;
      pixel.blue += oleft.blue;
      if(degrees < 0.0){
        if ((y + skewi) > 0)
        {
          int new_y = (y + skewi);
          if(pixel.red > 255) pixel.red = 255;
          if(pixel.green > 255) pixel.green = 255;
          if(pixel.blue > 255) pixel.blue = 255;
          set_pixel(sheared, &x, &new_y, pixel.red, pixel.green, pixel.blue);
          if(pixel.red != ignore_r || pixel.green != ignore_g || pixel.blue != ignore_b){
            if(new_y < min_y) min_y = new_y;
            if(new_y > max_y) max_y = new_y;
            if(x < min_x) min_x = x;
            if(x > max_x) max_x = x;
          }
        }
        oleft = left;
      }
      else{
        if ((y+skewi + ((dim_y) - (*board) -> resolution_y)) > 0 )
        {
          int new_y = (y+skewi + ((dim_y) - (*board) -> resolution_y));
          if(pixel.red > 255) pixel.red = 255;
          if(pixel.green > 255) pixel.green = 255;
          if(pixel.blue > 255) pixel.blue = 255;
          set_pixel(sheared, &x, &new_y, pixel.red, pixel.green, pixel.blue);
          if(pixel.red != ignore_r || pixel.green != ignore_g || pixel.blue != ignore_b){
            if(new_y < min_y) min_y = new_y;
            if(new_y > max_y) max_y = new_y;
            if(x < min_x) min_x = x;
            if(x > max_x) max_x = x;
          }
        }
        oleft = left;
      }
    }
  }
  int smaller_y = max_y - min_y+1;
  int smaller_x = max_x - min_x+1;
  struct board* resized_board = make_board(&smaller_x,&smaller_y);
  for(int c1 = 0; c1 < smaller_x; c1++){
    for(int c2 = 0; c2 < smaller_y; c2++){
      int altered_x = min_x + c1;
      int altered_y = min_y + c2;
      struct pixel old = get_pixel(sheared, &altered_x, &altered_y);
      set_pixel(resized_board,&c1,&c2, old.red, old.green, old.blue);
    }
  }
  free_board(board);
  free_board(&sheared);
  *board = resized_board;

  return 0;
}

int rotate(struct board** board, double degrees,int ignore_r, int ignore_g, int ignore_b)
{
  if(shear_x(board,degrees,ignore_r,ignore_g,ignore_b)==-1)
  {
    return -1;
  }
  if(shear_y(board,degrees,ignore_r,ignore_g,ignore_b)==-1)
  {
    return -1;
  }
  if(shear_x(board,degrees,ignore_r,ignore_g,ignore_b)==-1)
  {
    return -1;
  }
  return 0;
}

int resize_percent(struct board** board, double percent)
{
  int dim_x = ceil((*board)->resolution_x*percent);
  int dim_y = ceil((*board)->resolution_y*percent);

  struct board* resize = make_board(&dim_x,&dim_y);

  if(resize==NULL)
  {
    return -1;
  }

  struct pixel pixel;

  for (int y = 0; y < dim_y; y++)
  {
    for (int x = 0; x < dim_x; x++)
    {
      int pos_x = x*(1.0/percent);
      int pos_y = y*(1.0/percent);

      pixel = get_pixel(*board,&pos_x,&pos_y);
      set_pixel(resize,&x,&y,pixel.red,pixel.green,pixel.blue);
    }
  }

  free_board(board);
  *board = resize;

  return 0;
}
int resize_dimension(struct board** board, int dim_x, int dim_y)
{
  double y_percent= (double)(*board)->resolution_y/(double)dim_y;
  double x_percent = (double)(*board)->resolution_x/(double)dim_x;

  struct board* resize = make_board(&dim_x,&dim_y);

  if(resize==NULL)
  {
    return -1;
  }

  struct pixel pixel;

  for (int y = 0; y < dim_y; y++)
  {
    for (int x = 0; x < dim_x; x++)
    {
      int pos_x = x*(x_percent);
      int pos_y = y*(y_percent);

       pixel = get_pixel(*board,&pos_x,&pos_y);
       set_pixel(resize,&x,&y,pixel.red,pixel.green,pixel.blue);
    }
  }

  free_board(board);
  *board = resize;

  return 0;
}

//returns the average pixel color
//needed for hashing
//should happen after resizing if resizing
int to_grayscale(struct board** board)
{
  int avg_color = 0;

  struct pixel pixel;
  for (int y = 0; y < (*board)->resolution_y; y++)
  {
    for (int x = 0; x < (*board)->resolution_x; x++)
    {
      pixel = get_pixel(*board,&x,&y);
      int gray = (pixel.red+pixel.blue+pixel.green)/3;
      avg_color+=gray;
      set_pixel(*board,&x,&y,gray,gray,gray);
    }
  }
  return avg_color / ((*board)->resolution_x*(*board)->resolution_y);
}

void bounding_box(struct board** board, struct best_score_info* score)
{
  int start_x = score->search_start_x;
  int start_y = score->search_start_y;
  int bounding_box_x = score->dimension_x;
  int bounding_box_y = score->dimension_y;

	//change maybe?
	int border_thickness = 3;
	int x = 0;
	int y = 0;

	struct hsv hsv;
	hsv.h=0;
	hsv.s=1;
	hsv.v=1;

	int r=0;
	int g=0;
	int b=0;

	double adder = 6.0/(double)bounding_box_x;
	//double adder = (double)bounding_box_x/360.0;
	printf("The adder is %f\n",adder);

  int original_dimx = (*board) -> resolution_x;
  int original_dimy = (*board) -> resolution_y;

	//horizontal
	for (x = 0; x < bounding_box_x; ++x)
	{
		HSVtoRGB(hsv,&r,&g,&b);
		int border_x = x+start_x;
		int border_x2 = bounding_box_x-x+start_x;
		for(int thick = 0; thick < border_thickness; ++thick)
		{
			int border_y = thick+start_y;
      if(border_y<(*board)->resolution_y && border_x<(*board)->resolution_x)
      {
		    set_pixel(*board,&border_x,&border_y,r,g,b);
      }
			border_y = thick+start_y+bounding_box_y;
      if(border_y<(*board)->resolution_y && border_x2<(*board)->resolution_x)
      {
			  set_pixel(*board,&border_x2,&border_y,r,g,b);
      }
		}
		hsv.h+=adder;
	}

	//vertical
	adder = 6.0/(double)bounding_box_y;
	for (y = 0; y < bounding_box_y+border_thickness; ++y)
	{
		HSVtoRGB(hsv,&r,&g,&b);
		int border_y = y+start_y;
		int border_y2 = bounding_box_y-y+start_y+border_thickness-1;
		for(int thick = 0; thick < border_thickness; ++thick)
		{
			int border_x = thick+start_x;
      if(border_y2<(*board)->resolution_y && border_x<(*board)->resolution_x)
      {
		    set_pixel(*board,&border_x,&border_y2,r,g,b);
      }
			border_x = thick+start_x+bounding_box_x;
      if(border_y<(*board)->resolution_y && border_x<(*board)->resolution_x)
      {
		    set_pixel(*board,&border_x,&border_y,r,g,b);
      }
		}
		hsv.h+=adder;
	}
}
