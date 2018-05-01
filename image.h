#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include "score.h"

#define MAX_COLOR 255
#define DEFAULT_COLOR 0
#define PI 3.14159265358979323846

//color struct represents a pixel.
struct pixel{
  //change to uint later
  int red;
  int green;
  int blue;
};


//top left of board is 0,0
struct board
{
  int resolution_x;
  int resolution_y;
  struct pixel** image;
};

struct pixel get_pixel(const struct board* board, const int* x, const int* y);
void set_pixel(const struct board *board, const int *x, const int *y, int r, int g, int b);
void scale_pixel(struct pixel* pixel, double scale);
void save_ppm(const struct board* board, const char* file);
struct board* make_board(const int* res_x, const int* res_y);
struct board *copy_board(const struct board* old_board);
void free_board(struct board** board);
struct board* load_ppm(const char* file);
int autocrop_board(struct board** board, int ignore_r, int ignore_g, int ignore_b);
int shear_x(struct board** board, double degrees, int ignore_r, int ignore_g, int ignore_b);
int shear_y(struct board **board, double degrees, int ignore_r, int ignore_g, int ignore_b);
int rotate(struct board** board, double degrees,int ignore_r, int ignore_g, int ignore_b);
int resize_percent(struct board** board, double percent);
int resize_dimension(struct board** board, int dim_x, int dim_y);
int to_grayscale(struct board** board);
void bounding_box(struct board** board, struct best_score_info* score);

#endif