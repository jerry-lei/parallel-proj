#ifndef SCORE_H
#define SCORE_H

#include "image.h"
#include "boolean.h"
#include "hash.h"


struct best_score_info {
  double score;
  int search_start_x;
  int search_start_y;
  double avg_distance_from_closest_point;
  int total_hits;
};

int calc_distance(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                 int search_dimx, int search_dimy, int search_start_x, int search_start_y,
                 int pos_x, int pos_y);

struct best_score_info* calc_score(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y);

struct best_score_info* calc_best_score(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy);

#endif
