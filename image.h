#ifndef IMAGE_H
#define IMAGE_H

//http://datagenetics.com/blog/august32013/index.html
//https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html

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
void free_board(struct board** board);
struct board* load_ppm(const char* file);
int resize_board(struct board** board, int ignore_r, int ignore_g, int ignore_b);
int shear_x_experiment(struct board** board, double degrees);
int shear_y(struct board** board, double degrees);
int shear_y_experiment(struct board **board, double degrees);
#endif
