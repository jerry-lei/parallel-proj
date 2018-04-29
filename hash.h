#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>


#include "image.h"
#include "score.h"
#include "hsv.h"

struct hsv_hash{
  uint64_t h;
  uint64_t s;
  uint64_t v;
  double avg_hue;
  struct hsv corner;
  double hash2;
};

int hamming_distance(uint64_t* hash1, uint64_t* hash2);

struct best_score_info find_image(struct board** original_image, struct board** search_image, double scale);

struct best_score_info hash_thread_allocator(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y);
//void split_hash_HSV(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y);
struct hsv_hash** hash_original_HSV(struct board** original_image,int* original_dim_x, int* original_dim_y);
void hash_worker(struct hsv_hash **original_hashed_image, struct pixel **my_search_image,
								int **hitbox, int original_dim_x, int original_dim_y,
								int search_dim_x, int search_dim_y, int start_x, int start_y,
								pthread_mutex_t* hitbox_mutex, int total_threads);
void* call_thread(void* args);
struct hsv_hash hash8_hsv_pixels(struct pixel** board,int start_x, int start_y);
int sky_filter(struct hsv* corner);
#endif
