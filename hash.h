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
struct color_hash hash8_avg_color_pixels(struct pixel **board, int start_x, int start_y);
struct color_hash** hash_avg_original_color(struct board** original_image,int* original_dim_x, int* original_dim_y);
void split_avg_hash_color(struct board **search_image, struct color_hash** original_hashed_image, int original_dim_x, int original_dim_y);
void *thread_avg_hash_color(void *args);
#endif
