#include "hsv.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double min(double x, double y)
{
  return (x < y) ? x : y;
}

double max(double x, double y)
{
  return (x > y) ? x : y;
}


struct hsv RGBtoHSV(int r, int g, int b){
  double in_r = r/255.0;
  double in_g = g/255.0;
  double in_b = b/255.0;
  struct hsv returnHSV;
  double maximum = max(in_r,(max(in_g,in_b)));
  double minimum = min(in_r,(min(in_g,in_b)));
  double chroma = maximum - minimum;
  double hp, sp, vp;
  //hue
  if(maximum == in_r){
    hp = fmod(((in_g-in_b)/chroma),6);
  }
  else if(maximum == in_g){
    hp = ((in_b-in_r)/chroma) + 2;
  }
  else if(maximum == in_b){
    hp = ((in_r-in_g)/chroma) + 4;
  }
  if(chroma > 0.000000001){
    returnHSV.h = hp * 60;
  }
  else{ //chroma == 0
    returnHSV.h = 0;
  }
  if(returnHSV.h < 0){
    returnHSV.h += 360;
  }

  //saturation:
  if(maximum == 0) returnHSV.s = 0;
  else returnHSV.s = chroma/maximum;

  //value:
  returnHSV.v = maximum;

  return returnHSV;
}
