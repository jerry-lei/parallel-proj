#ifndef HASH_H
#define HASH_H

#include "image.h"
#include "boolean.h"
#include <stdint.h>


void * thread_function(void * args);
void split_hash(struct board** search_image,uint64_t** original_hashed_image, int original_dim_x, int original_dim_y);
uint64_t hash8_gray(struct board** board, int color_avg);
int hamming_distance(uint64_t* hash1, uint64_t* hash2);

#endif
