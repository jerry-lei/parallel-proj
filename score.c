
#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <inttypes.h>

int max_int(int x, int y){
  return (x > y) ? x : y;
}

int min_int(int x, int y){
  return (x < y) ? x : y;
}


//diagram in case i forget: https://photos.app.goo.gl/hQTDLXTIxD9PWgj53
int calc_distance(int** hitbox, int** distance_box, int hitbox_dimx, int hitbox_dimy,
                 int search_dimx, int search_dimy, int search_start_x, int search_start_y,
                 int pos_x, int pos_y)
{
  int current_min = distance_box[pos_y][pos_x];
  int max_distance = max_int(search_dimx, search_dimy);
  for(int n = 1; n < min_int(max_distance, current_min); n++){
    //search: top
    if((pos_y - n) > 0 && (pos_y - n) > search_start_y){
      //get bounds for starting position / ending position
      int find_start = max_int(0, max_int(search_start_x, (pos_x - n)));
      int find_end = min_int(hitbox_dimx, min_int((search_start_x + search_dimx), (pos_x + n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[pos_y-n][c1] != 0) {
          distance_box[pos_y-n][c1] = n;
          distance_box[pos_y][c1] = n;
          return n;
        }
      }
    }
    //search: bottom
    if((pos_y + n) < hitbox_dimy && (pos_y + n) < (search_start_y + search_dimy)){
      int find_start = max_int(0, max_int(search_start_x, (pos_x - n)));
      int find_end = min_int(hitbox_dimx, min_int((search_start_x + search_dimx), (pos_x + n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[pos_y+n][c1] != 0) {
          distance_box[pos_y+n][c1] = n;
          distance_box[pos_y][c1] = n;
          return n;
        }
      }
    }
    //search left.
    if((pos_x - n) > 0 && (pos_x - n) > search_start_x){
      //repeating corner searches for easier logic.
      int find_start = max_int(0, max_int(search_start_y, (pos_y-n)));
      int find_end = min_int(hitbox_dimy, min_int((search_start_y + search_dimy), (pos_y+n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[c1][pos_x-n] != 0) {
          distance_box[c1][pos_x-n] = n;
          distance_box[c1][pos_x] = n;
          return n;
        }
      }
    }
    //search right
    if((pos_x + n) < hitbox_dimx && (pos_x + n) < (search_start_x + search_dimx)){
      //repeating corner searches for easier logic.
      int find_start = max_int(0, max_int(search_start_y, (pos_y-n)));
      int find_end = min_int(hitbox_dimy, min_int((search_start_y + search_dimy), (pos_y+n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[c1][pos_x+n] != 0) {
          distance_box[c1][pos_x+n] = n;
          distance_box[c1][pos_x] = n;
          return n;
        }
      }
    }
  }
  return min_int(max_distance,current_min);
}

struct best_score_info calc_score(int** hitbox, int** distance_box, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y)
{
  int total_distance = 0;
  int total_hits = 0;
  for(int row = 0; row < search_dimy; row++){
    for(int col = 0; col < search_dimx; col++){
      if(hitbox[row+search_start_y][col+search_start_x] != 0){
        int distance = calc_distance(hitbox, distance_box, hitbox_dimx, hitbox_dimy, search_dimx, search_dimy, search_start_x, search_start_y, col + search_start_x, row + search_start_y);
        total_distance += distance;
        total_hits += 1;
      }
    }
  }
  double average_distance = total_distance/(double)total_hits;
  double inverted_avg_distance = 1.0/average_distance;
  int score = total_hits;///inverted_avg_distance * total_hits;
  // if(total_hits > 1)
  //   printf("Total: %f\n", score);
  struct best_score_info return_score_info;
  return_score_info.score = score;
  return_score_info.search_start_x = search_start_x;
  return_score_info.search_start_y = search_start_y;
  return_score_info.avg_distance_from_closest_point = average_distance;
  return_score_info.total_hits = (int)total_hits;
  return return_score_info;
}


struct best_score_info calc_best_score(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy)
{
  struct best_score_info curr_best;
  curr_best.score = -1;
  int** distance_box = malloc(hitbox_dimy * sizeof(int*));
  for(int c1 = 0; c1 < hitbox_dimy; c1++){
    distance_box[c1] = malloc(hitbox_dimx * sizeof(int));
    for(int c2 = 0; c2< hitbox_dimx; c2++){
      distance_box[c1][c2] = INT_MAX;
    }
  }

  for(int row = 0; row < hitbox_dimy - search_dimy; row++){
    for(int col = 0; col < hitbox_dimx - search_dimx; col++){
      //printf("Row: %d, col: %d\n", row, col);
      struct best_score_info check_score = calc_score(hitbox, distance_box, hitbox_dimx, hitbox_dimy, search_dimx, search_dimy, col, row);
      if(check_score.score > curr_best.score){
        curr_best.score = check_score.score;
        curr_best.search_start_x = check_score.search_start_x;
        curr_best.search_start_y = check_score.search_start_y;
        curr_best.avg_distance_from_closest_point = check_score.avg_distance_from_closest_point;
        curr_best.total_hits = check_score.total_hits;

      }
    }
  }
  return curr_best;
}
