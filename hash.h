#ifndef HASH_H
#define HASH_H

#include "image.h"
#include "boolean.h"
#include <stdint.h>
#include <inttypes.h>

int hamming_distance(uint64_t* hash1, uint64_t* hash2);
void * thread_hash(void * args);
void split_hash(struct board** search_image,uint64_t** original_hashed_image, int original_dim_x, int original_dim_y);
uint64_t** hash_original(struct board** original_image,int* original_dim_x, int* original_dim_y);
uint64_t hash8_gray_board(struct board** board);
uint64_t hash8_gray_pixels(struct pixel** board, int start_x, int start_y);

#endif
