#ifndef IMAGE_H
#define IMAGE_H

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
void save_ppm(const struct board* board, const char* file);
struct board* make_board(const int* res_x, const int* res_y);
void free_board(struct board** board);
struct board* load_ppm(const char* file);
int sheer(struct board** board, double degrees);
#endif



//http://datagenetics.com/blog/august32013/index.html