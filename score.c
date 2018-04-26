


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
  int current_min = current.distance;
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

  return current_min;
}

struct best_score_info calc_score(int** hitbox, struct opt_dist** distance_box, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y)
{
  int total_distance = 0;
  int total_hits = 0;
  int max_distance = -1;
  int min_distance = INT_MAX;
  double from_top_left = DBL_MAX;
  double from_top_right = DBL_MAX;
  double from_bottom_left = DBL_MAX;
  double from_bottom_right = DBL_MAX;
  double total_distance_from_center = 0;
  double center_x = search_start_x + search_dimx/2.0;
  double center_y = search_start_y + search_dimy/2.0;
  for(int row = search_start_y; row < min_int(search_dimy + search_start_y, hitbox_dimy); row++){
    for(int col = search_start_x; col < min_int(search_dimx + search_start_x, hitbox_dimx); col++){
      //if(search_start_x == 0 && search_start_y == 1) printf("2: Row: %d, Col %d\n", row, col);
      if(hitbox[row][col] != 0){
        int check_dist = distance_box[row][col].distance;
        total_distance += distance(center_x, center_y, col, row);
        double check_tl = distance(search_start_x,search_start_y,col,row);
        double check_tr = distance(search_start_x + search_dimx-1,search_start_y,col,row);
        double check_bl = distance(search_start_x,search_start_y + search_dimy-1,col,row);
        double check_br = distance(search_start_x + search_dimx-1,search_start_y + search_dimy-1,col,row);
        if(check_tl < from_top_left) from_top_left = check_tl;
        if(check_tr < from_top_right) from_top_right = check_tr;
        if(check_bl < from_bottom_left) from_bottom_left = check_bl;
        if(check_br < from_bottom_right) from_bottom_right = check_br;
        if(check_dist > max_distance) max_distance = check_dist;
        if(check_dist < min_distance) min_distance = check_dist;
        total_hits += 1;
      }
    }
  }
  //printf("TL: %f, TR: %f, BL: %f, BR: %f\n", from_top_left, from_top_right, from_bottom_left, from_bottom_right);
  double average_distance = total_distance/(double)total_hits;
  int range = max_distance - min_distance;
  double score = -1;
  double inverted_range = -1;
  double total_corner_distance = from_top_left + from_top_right + from_bottom_left + from_bottom_right;
  double avg_min_distance_from_corners = total_corner_distance/4;
  double inverted_avg_min_distance_from_corners = -1;
  double inverted_avg_distance = 1.0/average_distance;
  double inverted_max_distance = -1;
  double inverted_min_distance = -1;
  double midpoint_corner_center_x = center_x + search_dimx/4.0;
  double midpoint_corner_center_y = center_y + search_dimy/4.0;
  double opt_avg_distance_from_corners = distance(center_x, center_y, midpoint_corner_center_x, midpoint_corner_center_y);
  if(range != 0 && avg_min_distance_from_corners > 0 && total_hits > 0){
    double avg_distance_from_center = total_distance_from_center/total_hits;
    double delta_avg_opt_distance_from_center = fabs(avg_distance_from_center - opt_avg_distance_from_corners);
    double inverted_delta_avg_opt_distance_from_center = 1.0/delta_avg_opt_distance_from_center;
    inverted_range = 1.0/range;
    inverted_max_distance = 1.0/max_distance;
    inverted_min_distance = 1.0/min_distance;
    inverted_avg_min_distance_from_corners = 1.0/avg_min_distance_from_corners;
    double inverted_dimensions = 1.0/(search_dimx * search_dimy);
    score = total_hits * total_hits * inverted_max_distance * inverted_delta_avg_opt_distance_from_center * inverted_dimensions;
  }
  // if(total_hits > 0){
  //   score = (double)total_hits/(search_dimx * search_dimy);
  // }
  struct best_score_info return_score_info;
  return_score_info.score = score;
  return_score_info.search_start_x = search_start_x;
  return_score_info.search_start_y = search_start_y;
  return_score_info.extra_info = max_distance;
  return_score_info.total_hits = (int)total_hits;
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
    for(int c2 = 0; c2< original_dimx; c2++){
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
      }
    }
  }

  curr_best.dimension_x = min_int(original_dimx, search_dimx);
  curr_best.dimension_y = min_int(original_dimy, search_dimy);

  for(int c1 = 0; c1 < original_dimy; c1++){
    free(distance_box[c1]);
  }
  free(distance_box);


  return curr_best;
}
