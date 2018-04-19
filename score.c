
#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

int max_int(int x, int y){
  return (x > y) ? x : y;
}

int min_int(int x, int y){
  return (x < y) ? x : y;
}


//diagram in case i forget: https://photos.app.goo.gl/hQTDLXTIxD9PWgj53
int calc_distance(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                 int search_dimx, int search_dimy, int search_start_x, int search_start_y,
                 int pos_x, int pos_y)
{
  int max_distance = max_int(search_dimx, search_dimy);
  for(int n = 1; n < max_distance; n++){
    //search: top
    if((pos_y - n) > 0 && (pos_y - n) > search_start_y){
      //get bounds for starting position / ending position
      int find_start = max_int(0, max_int(search_start_x, (pos_x - n)));
      int find_end = min_int(hitbox_dimx, min_int((search_start_x + search_dimx), (pos_x + n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[pos_y-n][c1] != 0) return n;
      }
    }
    //search: bottom
    if((pos_y + n) < hitbox_dimy && (pos_y + n) < (search_start_y + search_dimy)){
      int find_start = max_int(0, max_int(search_start_x, (pos_x - n)));
      int find_end = min_int(hitbox_dimx, min_int((search_start_x + search_dimx), (pos_x + n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[pos_y+n][c1] != 0) return n;
      }
    }
    //search left.
    if((pos_x - n) > 0 && (pos_x - n) > search_start_x){
      //repeating corner searches for easier logic.
      int find_start = max_int(0, max_int(search_start_y, (pos_y-n)));
      int find_end = min_int(hitbox_dimy, min_int((search_start_y + search_dimy), (pos_y+n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[c1][pos_x-n] != 0) return n;
      }
    }
    //search right
    if((pos_x + n) < hitbox_dimx && (pos_x + n) < (search_start_x + search_dimx)){
      //repeating corner searches for easier logic.
      int find_start = max_int(0, max_int(search_start_y, (pos_y-n)));
      int find_end = min_int(hitbox_dimy, min_int((search_start_y + search_dimy), (pos_y+n)));
      for(int c1 = find_start; c1 < find_end; c1++){
        if(hitbox[c1][pos_x+n] != 0) return n;
      }
    }
  }
  return max_distance;
}

struct best_score_info calc_score(int** hitbox, int hitbox_dimx, int hitbox_dimy,
                int search_dimx, int search_dimy, int search_start_x, int search_start_y)
{
  int total_distance = 0;
  double total_hits = 0;
  for(int row = 0; row < search_dimy; row++){
    for(int col = 0; col < search_dimx; col++){
      if(hitbox[row][col] != 0){
        int distance = calc_distance(hitbox, hitbox_dimx, hitbox_dimy, search_dimx, search_dimy, search_start_x, search_start_y, col + search_start_x, col + search_start_y);
        total_distance += distance;
        total_hits += 1.0;
      }
    }
  }
  double average_distance = total_distance/total_hits;
  double inverted_avg_distance = 1.0/average_distance;
  double score = total_hits;///inverted_avg_distance * total_hits;
  //printf("Score: %f\n", score);
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
  for(int col = 0; col < hitbox_dimy - search_dimy; ++col){
    for(int row = 0; row < hitbox_dimx - search_dimx; ++row){
      struct best_score_info check_score = calc_score(hitbox, hitbox_dimx, hitbox_dimy, search_dimx, search_dimy, row, col);
      if(check_score.score > curr_best.score){
        curr_best = check_score;
      }
    }
  }
  return curr_best;
}
