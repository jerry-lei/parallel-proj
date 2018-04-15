#ifndef HSV_H
#define HSV_H
#include <stdint.h>


struct hsv{
  double h;
  double s;
  double v;
};

inline double max(double x, double y);
inline double min(double x, double y);

struct hsv RGBtoHSV(int r, int g, int b);


#endif
