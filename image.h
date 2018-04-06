#ifndef IMAGE_H
#define IMAGE_H

#define MAX_COLOR 255
#define DEFAULT_COLOR 0

//color struct represents a pixel.
struct pixel{
  //change to uint later
  int red;
  int green;
  int blue;
};

struct board
{
  int resolution_x;
  int resolution_y;
  struct pixel** image;
};

struct pixel get_pixel(const struct board* board, const int* x, const int* y);
void set_pixel(const struct board* board, const int* x, const int* y, const int* r, const int* g, const int* b);
void save_ppm(const struct board* board, const char* file);
*board make_board(const int* res_x, const int* res_y);



#endif
