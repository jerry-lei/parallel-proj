#ifndef SCORE_H
#define SCORE_H

#include "image.h"
#include "boolean.h"
#include "hash.h"

struct opt_dist{
  int set_itself;
  int distance;
};

struct best_score_info {
  double score;
  int search_start_x;
  int search_start_y;
  double extra_info;
  int total_hits;
};

double distance(double x1, double y1, double x2, double y2);

int calc_distance(int** hitbox, struct opt_dist** distance_box, int hitbox_dimx, int hitbox_dimy,
                 int search_dimx, int search_dimy, int search_start_x, int search_start_y,
                 int pos_x, int pos_y);

struct best_score_info calc_score(int** hitbox, struct opt_dist** distance_box, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y);

struct best_score_info calc_best_score(int** hitbox, int original_dimx, int original_dimy,
                int search_dimx, int search_dimy);

#endif
