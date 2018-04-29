#ifndef SCORE_H
#define SCORE_H
//#include "image.h"

#define NUMBER_BUCKETS 6


struct best_score_info {
  double score;
  int search_start_x;
  int search_start_y;
  int dimension_x;
  int dimension_y;
  double extra_info;
  int total_hits;
};


int max_int(int x, int y);

int min_int(int x, int y);

double distance(double x1, double y1, double x2, double y2);


struct best_score_info calc_score(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y, double** optimal_distribution_x, double** optimal_distribution_y);

struct best_score_info calc_best_score(int** hitbox, int original_dimx, int original_dimy,
                int search_dimx, int search_dimy, double** optimal_distribution_x, double** optimal_distribution_y);

#endif
