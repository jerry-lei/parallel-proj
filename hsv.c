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

void HSVtoRGB (struct hsv hsv, int* r_out, int* g_out, int* b_out)
{
  double c = hsv.v*hsv.s;
  double hp = fmod(hsv.h,6);
  double abs_hp =fmod(hp,2)-1;
  if(abs_hp<0){abs_hp*=-1;}
  double x = c*(1.0-abs_hp);
  double r,g,b;

  if(hp>=0&& hp<1)
  {
    r=c;g=x;b=0;
  }
  else if(hp>=1&& hp<2)
  {
    r=x;g=c;b=0;
  }
  else if(hp>=2&& hp<3)
  {
    r=0;g=c;b=x;
  }
  else if(hp>=3&& hp<4)
  {
    r=0;g=x;b=c;
  }
  else if(hp>=4&& hp<5)
  {
    r=x;g=0;b=c;
  }else if(hp>=5&& hp<6)
  {
    r=c;g=0;b=x;
  }else
  {
    r=0;g=0;b=0;
  }
  double m = hsv.v-c;

  *r_out = 255*(r+m);
  *g_out = 255*(g+m);
  *b_out = 255*(b+m);
}
