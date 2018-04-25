#ifndef HASH_H
#define HASH_H

#include "image.h"
#include "boolean.h"
#include "score.h"
#include <stdint.h>
#include <inttypes.h>


struct hsv_hash{
  uint64_t h;
  uint64_t s;
  uint64_t v;
  double avg_hue;
  double corner_hue;
  double hash2;
};

int hamming_distance(uint64_t* hash1, uint64_t* hash2);


//void * thread_hash_HSV(void * args);
void hash_thread_allocator(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y);
//void split_hash_HSV(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y);
struct hsv_hash** hash_original_HSV(struct board** original_image,int* original_dim_x, int* original_dim_y);
void* hash_worker(void* args);
struct hsv_hash hash8_hsv_pixels(struct pixel** board,int start_x, int start_y);
#endif
