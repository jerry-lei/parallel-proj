#ifndef HASH_H
#define HASH_H

#include "image.h"
#include "boolean.h"
#include <stdint.h>
#include <inttypes.h>

struct color_hash{
  uint64_t red;
  uint64_t green;
  uint64_t blue;
};

struct hsv_hash{
  uint64_t h;
  uint64_t s;
  uint64_t v;
  double avg_hue;
  double corner_hue;
  double hash2;
};

int hamming_distance(uint64_t* hash1, uint64_t* hash2);
void * thread_hash_gray(void * args);
void * thread_hash_color(void * args);
void split_hash_gray(struct board** search_image,uint64_t** original_hashed_image, int original_dim_x, int original_dim_y);
void split_hash_color(struct board** search_image,struct color_hash** original_hashed_image, int original_dim_x, int original_dim_y);
uint64_t** hash_original_gray(struct board** original_image,int* original_dim_x, int* original_dim_y);
struct color_hash** hash_original_color(struct board** original_image,int* original_dim_x, int* original_dim_y);
uint64_t hash8_gray_board(struct board** board);
uint64_t hash8_gray_pixels(struct pixel** board, int start_x, int start_y);
struct color_hash hash8_color_pixels(struct pixel** board, int start_x, int start_y);




void * thread_hash_HSV(void * args);
void split_hash_HSV(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y);
struct hsv_hash** hash_original_HSV(struct board** original_image,int* original_dim_x, int* original_dim_y);
struct hsv_hash hash8_hsv_pixels(struct pixel** board,int start_x, int start_y);
#endif
