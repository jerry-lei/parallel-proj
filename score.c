#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <inttypes.h>
#include <float.h>

#include "score.h"

int max_int(int x, int y){
  return (x > y) ? x : y;
}

int min_int(int x, int y){
  return (x < y) ? x : y;
}


double distance(double x1, double y1, double x2, double y2){
  return sqrt(((x2-x1) * (x2-x1)) + ((y2-y1) * (y2-y1)));
}


struct best_score_info calc_score(int** hitbox,  int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y, double** optimal_distribution_x, double** optimal_distribution_y)
{
  /**
  Metrics:
    - Total hits (higher = better)
    - Density (Total hits / (search_dimx * search_dimy)) (higher = better)
    - Maximum distance of all hits to its nearest neighbor. (Lower = better)
    - Average distance from the center. (Lower = better)
  **/
  int total_hits = 0;
  int unique_hits = 0;
  //total_x, total_y is gets incremented by values from [0->search_start_(x/y)]
  int total_x_positions = 0;
  int total_y_positions = 0;
  double *bucket_x = calloc(NUMBER_BUCKETS, sizeof(double));
  double *bucket_y = calloc(NUMBER_BUCKETS, sizeof(double));
  int bucket_width_x = ceil((double)search_dimx / NUMBER_BUCKETS);
  int bucket_width_y = ceil((double)search_dimy / NUMBER_BUCKETS);
  int total_bucket_hits = 0;
  for(int row = search_start_y; row < min_int(search_dimy + search_start_y, hitbox_dimy); ++row){
    for(int col = search_start_x; col < min_int(search_dimx + search_start_x, hitbox_dimx); ++col){
      if(hitbox[row][col] != 0){
        total_hits += hitbox[row][col];
        //look into multiplying by 64
        bucket_x[(col-search_start_x)/bucket_width_x] += (64*hitbox[row][col]);
        bucket_y[(row-search_start_y)/bucket_width_y] += (64*hitbox[row][col]);
        total_bucket_hits += (64*hitbox[row][col]);
        unique_hits += 1;
      }
    }
  }

  double density = (double)total_hits/(search_dimx * search_dimy);

  for(int c1 = 0; c1 < NUMBER_BUCKETS; c1++){
    bucket_x[c1] /= (total_bucket_hits);
    bucket_y[c1] /= (total_bucket_hits);
  }

  double difference_x = 0.0;
  double difference_y = 0.0;
  //double opt_distribution
  for(int c1 = 0; c1 < NUMBER_BUCKETS; c1++){
    difference_x += fabs(bucket_x[c1] - (*optimal_distribution_x)[c1]);
    difference_y += fabs(bucket_y[c1] - (*optimal_distribution_y)[c1]);
  }



  double score = 1.0;
  score /= (difference_x * difference_y);
  score *= density;



  struct best_score_info return_score_info;
  return_score_info.score = score;
  return_score_info.score = score;
  return_score_info.search_start_x = search_start_x;
  return_score_info.search_start_y = search_start_y;
  return_score_info.extra_info = 0.0;
  return_score_info.total_hits = (int)total_hits;

  //FREE THE BUCKETS
  free(bucket_x);
	free(bucket_y);

  return return_score_info;
}


struct best_score_info calc_best_score(int** hitbox, int original_dimx, int original_dimy,
                int search_dimx, int search_dimy, double** optimal_distribution_x, double** optimal_distribution_y)
{
  struct best_score_info curr_best;
  curr_best.score = -1;
  curr_best.search_start_x = -1;
  curr_best.search_start_y = -1;
  curr_best.extra_info = -1;
  curr_best.total_hits = -1;


  for(int row = 0; row < min_int(original_dimy, max_int(original_dimy - search_dimy, search_dimy)); row++){
    for(int col = 0; col < min_int(original_dimx, max_int(original_dimx - search_dimx, search_dimx)); col++){
      struct best_score_info check_score = calc_score(hitbox, original_dimx, original_dimy, search_dimx, search_dimy, col, row, optimal_distribution_x, optimal_distribution_y);
      if(check_score.score > curr_best.score){
        curr_best.score = check_score.score;
        curr_best.search_start_x = check_score.search_start_x;
        curr_best.search_start_y = check_score.search_start_y;
        curr_best.extra_info = check_score.extra_info;
        curr_best.total_hits = check_score.total_hits;
      }
    }
  }

  curr_best.dimension_x = min_int(original_dimx, search_dimx);
  curr_best.dimension_y = min_int(original_dimy, search_dimy);

  return curr_best;
}
