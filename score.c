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


//diagram in case i forget: https://photos.app.goo.gl/hQTDLXTIxD9PWgj53
int calc_distance(int** hitbox, struct opt_dist** distance_box, int hitbox_dimx, int hitbox_dimy,
                 int pos_x, int pos_y)
{
  struct opt_dist current = distance_box[pos_y][pos_x];
  if(current.set_itself == 1){
    return current.distance;
  }
  int current_min = min_int(current.distance, min_int(hitbox_dimx, hitbox_dimy));
  for(int n = 1; n < current_min; n++){
    //search: top
    if((pos_y - n) >= 0){
      //get bounds for starting position / ending position
      int find_start = max_int(0,  (pos_x - n));
      int find_end = min_int(hitbox_dimx, (pos_x + n));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[pos_y-n][c1] != 0) {
          struct opt_dist change = distance_box[pos_y-n][c1];
          if(change.set_itself == 0){
            distance_box[pos_y-n][c1].distance = n;
          }
          distance_box[pos_y][pos_x].distance = n;
          distance_box[pos_y][pos_x].set_itself = 1;
          return n;
        }
      }
    }

    //search: bottom
    if((pos_y + n) < hitbox_dimy){
      int find_start = max_int(0, (pos_x - n));
      int find_end = min_int(hitbox_dimx, (pos_x + n));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[pos_y+n][c1] != 0) {
          struct opt_dist change = distance_box[pos_y+n][c1];
          if(change.set_itself == 0){
            distance_box[pos_y+n][c1].distance = n;
          }
          distance_box[pos_y][pos_x].distance = n;
          distance_box[pos_y][pos_x].set_itself = 1;
          return n;
        }
      }
    }
    //search left.
    if((pos_x - n) >= 0){
      //repeating corner searches for easier logic.
      int find_start = max_int(0, (pos_y-n));
      int find_end = min_int(hitbox_dimy, (pos_y+n));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[c1][pos_x-n] != 0) {
          struct opt_dist change = distance_box[c1][pos_x-n];
          if(change.set_itself == 0){
            distance_box[c1][pos_x-n].distance = n;
          }
          distance_box[pos_y][pos_x].distance = n;
          distance_box[pos_y][pos_x].set_itself = 1;
          return n;
        }
      }
    }
    //search right
    if((pos_x + n) < hitbox_dimx){
      //repeating corner searches for easier logic.
      int find_start = max_int(0, (pos_y-n));
      int find_end = min_int(hitbox_dimy, (pos_y+n));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[c1][pos_x+n] != 0) {
          struct opt_dist change = distance_box[c1][pos_x+n];
          if(change.set_itself == 0){
            distance_box[c1][pos_x+n].distance = n;
          }
          distance_box[pos_y][pos_x].distance = n;
          distance_box[pos_y][pos_x].set_itself = 1;
          return n;
        }
      }
    }
  }
  distance_box[pos_y][pos_x].distance = current_min;
  distance_box[pos_y][pos_x].set_itself = 1;
  return current_min;
}

struct best_score_info calc_score(int** hitbox, struct opt_dist** distance_box, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y)
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
  int max_distance = -1;
  //total_x, total_y is gets incremented by values from [0->search_start_(x/y)]
  int total_x_positions = 0;
  int total_y_positions = 0;
  int number_of_buckets = 6;
  double deviation_1 = 0.16666666;
  double deviation_2 = 0.16666666;
  double deviation_3 = 0.16666666;
  int *bucket_x = calloc(number_of_buckets, sizeof(int));
  int *bucket_y = calloc(number_of_buckets, sizeof(int));
  int bucket_width_x = ceil(search_dimx / number_of_buckets);
  int bucket_width_y = ceil(search_dimy / number_of_buckets);
  for(int row = search_start_y; row < min_int(search_dimy + search_start_y, hitbox_dimy); ++row){
    for(int col = search_start_x; col < min_int(search_dimx + search_start_x, hitbox_dimx); ++col){
      if(hitbox[row][col] != 0){
        total_hits += hitbox[row][col];
        bucket_x[(col-search_start_x)/bucket_width_x] += hitbox[row][col];
        bucket_y[(row-search_start_y)/bucket_width_y] += hitbox[row][col];
        unique_hits += 1;
        int check_nearest_distance = distance_box[row][col].distance;
        if(check_nearest_distance > max_distance) max_distance = check_nearest_distance;
        //updated by the weighted number of hits
        total_x_positions += hitbox[row][col] * (col - search_start_x);
        total_y_positions += hitbox[row][col] * (row - search_start_y);
      }
    }
  }

  //possible values are 0->search_dim(x,y)
  double average_position_x = (double)total_x_positions/total_hits;
  double average_position_y = (double)total_y_positions/total_hits;
  double center_position_x = (double)search_dimx/2.0;
  double center_position_y = (double)search_dimy/2.0;
  //printf("Avg_x: %f, Avg_y %f, Center_x: %f, Center_y: %f\n", average_position_x, average_position_y, center_position_x, center_position_y);
  //use above values to calculate aggregate distance from center point
  double average_distance_from_center_position = distance(average_position_x, average_position_y, center_position_x, center_position_y);
  double density = (double)total_hits/(search_dimx * search_dimy);
  //invert appropriate values
  double inverted_average_distance_from_center_position = 1.0/average_distance_from_center_position;
  if(average_distance_from_center_position < 0.00001) inverted_average_distance_from_center_position = 1.0/DBL_MAX;
  double inverted_max_distance = 1.0/max_distance;
  //calculate the score
  double corner2center_dist = distance(0,0,center_position_x,center_position_y);
  double score =  density * inverted_max_distance * inverted_average_distance_from_center_position * 10000;//total hits
  score/=(corner2center_dist);


  double* bucket_x_distribution = calloc(number_of_buckets, sizeof(double));
  double* bucket_y_distribution = calloc(number_of_buckets, sizeof(double));
  for(int c1 = 0; c1 < number_of_buckets; c1++){
    bucket_x_distribution[c1] = (double) bucket_x[c1] / total_hits;
    bucket_y_distribution[c1] = (double) bucket_y[c1] / total_hits;
  }
  double dist_neg_3_stddev_x = fabs(bucket_x_distribution[0] - deviation_3);
  double dist_neg_2_stddev_x = fabs(bucket_x_distribution[1] - deviation_2);
  double dist_neg_1_stddev_x = fabs(bucket_x_distribution[2] - deviation_1);
  double dist_pos_1_stddev_x = fabs(bucket_x_distribution[3] - deviation_1);
  double dist_pos_2_stddev_x = fabs(bucket_x_distribution[4] - deviation_2);
  double dist_pos_3_stddev_x = fabs(bucket_x_distribution[5] - deviation_3);

  double dist_neg_3_stddev_y = fabs(bucket_y_distribution[0] - deviation_3);
  double dist_neg_2_stddev_y = fabs(bucket_y_distribution[1] - deviation_2);
  double dist_neg_1_stddev_y = fabs(bucket_y_distribution[2] - deviation_1);
  double dist_pos_1_stddev_y = fabs(bucket_y_distribution[3] - deviation_1);
  double dist_pos_2_stddev_y = fabs(bucket_y_distribution[4] - deviation_2);
  double dist_pos_3_stddev_y = fabs(bucket_y_distribution[5] - deviation_3);

  double sum_diff_stddev_x = dist_neg_3_stddev_x + dist_neg_2_stddev_x + dist_neg_1_stddev_x + dist_pos_1_stddev_x + dist_pos_2_stddev_x + dist_pos_3_stddev_x;
  double sum_diff_stddev_y = dist_neg_3_stddev_y + dist_neg_2_stddev_y + dist_neg_1_stddev_y + dist_pos_1_stddev_y + dist_pos_2_stddev_y + dist_pos_3_stddev_y;

  double normalized_sum_diff_stddev_x = sum_diff_stddev_x;
  double normalized_sum_diff_stddev_y = sum_diff_stddev_y;

  score /= normalized_sum_diff_stddev_x;
  score /= normalized_sum_diff_stddev_y;


  struct best_score_info return_score_info;
  return_score_info.score = score;
  return_score_info.score = score;
  return_score_info.search_start_x = search_start_x;
  return_score_info.search_start_y = search_start_y;
  return_score_info.extra_info = average_distance_from_center_position;
  return_score_info.total_hits = (int)total_hits;


  //REMOVE LATER
  return_score_info.unique_hits=unique_hits;
  return_score_info.max_distance=max_distance;
  return_score_info.average_position_x=average_position_x;
  return_score_info.average_position_y=average_position_y;
  return_score_info.center_position_x=center_position_x;
  return_score_info.center_position_y=center_position_y;
  return_score_info.average_distance_from_center_position=average_distance_from_center_position;
  return_score_info.density=density;
  return_score_info.corner2center_dist=corner2center_dist;
  //////////////

  free(bucket_x);
  free(bucket_y);
  free(bucket_x_distribution);
  free(bucket_y_distribution);


  return return_score_info;

}


struct best_score_info calc_best_score(int** hitbox, int original_dimx, int original_dimy,
                int search_dimx, int search_dimy)
{
  struct best_score_info curr_best;
  curr_best.score = -1;
  curr_best.search_start_x = -1;
  curr_best.search_start_y = -1;
  curr_best.extra_info = -1;
  curr_best.total_hits = -1;
  struct opt_dist** distance_box = malloc(original_dimy * sizeof(struct opt_dist*));
  for(int c1 = 0; c1 < original_dimy; c1++){
    distance_box[c1] = malloc(original_dimx * sizeof(struct opt_dist));
    for(int c2 = 0; c2 < original_dimx; c2++){
      distance_box[c1][c2].distance = INT_MAX;
      distance_box[c1][c2].set_itself = 0;
    }
  }

  for(int row = 0; row < original_dimy; row++){
    for(int col = 0; col < original_dimx; col++){
      if(hitbox[row][col] != 0){
        calc_distance(hitbox, distance_box, original_dimx, original_dimy, col, row);
      }
    }
  }

  for(int row = 0; row < min_int(original_dimy, max_int(original_dimy - search_dimy, search_dimy)); row++){
    for(int col = 0; col < min_int(original_dimx, max_int(original_dimx - search_dimx, search_dimx)); col++){
      struct best_score_info check_score = calc_score(hitbox, distance_box, original_dimx, original_dimy, search_dimx, search_dimy, col, row);
      if(check_score.score > curr_best.score){
        curr_best.score = check_score.score;
        curr_best.search_start_x = check_score.search_start_x;
        curr_best.search_start_y = check_score.search_start_y;
        curr_best.extra_info = check_score.extra_info;
        curr_best.total_hits = check_score.total_hits;

        //REMOVE LATER
        curr_best.unique_hits=check_score.unique_hits;
        curr_best.max_distance=check_score.max_distance;
        curr_best.average_position_x=check_score.average_position_x;
        curr_best.average_position_y=check_score.average_position_y;
        curr_best.center_position_x=check_score.center_position_x;
        curr_best.center_position_y=check_score.center_position_y;
        curr_best.average_distance_from_center_position=check_score.average_distance_from_center_position;
        curr_best.density=check_score.density;
        curr_best.corner2center_dist=check_score.corner2center_dist;
        //////////////
      }
    }
  }

  curr_best.dimension_x = min_int(original_dimx, search_dimx);
  curr_best.dimension_y = min_int(original_dimy, search_dimy);





  //REMOVE LATER
  printf("%dx%d:\n  total_hits:%d\nunique_hits:%d\n max_dist:%d\n avgpos: %fx%f\n centerpos: %fx%f\n  avgdistfromcenter: %f\n density: %f\n corner2center: %f\n",
  curr_best.dimension_x,
  curr_best.dimension_y,
  curr_best.total_hits,
  curr_best.unique_hits,
  curr_best.max_distance,
  curr_best.average_position_x,
  curr_best.average_position_y,
  curr_best.center_position_x,
  curr_best.center_position_y,
  curr_best.average_distance_from_center_position,
  curr_best.density,
  curr_best.corner2center_dist);
  //////////////






  for(int c1 = 0; c1 < original_dimy; c1++){
    free(distance_box[c1]);
  }
  free(distance_box);


  return curr_best;
}
