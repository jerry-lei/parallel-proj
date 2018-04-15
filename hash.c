#include "hash.h"
#include "image.h"
#include "boolean.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <inttypes.h>

#define MAX_HAMMING 10
#define HASH_SIZE_X 9
#define HASH_SIZE_Y 8

struct search_thread_params {
  uint64_t** original_hashed_image;
  struct pixel** my_search_image;
  int** hitbox;
  int original_dim_x;
  int original_dim_y;
  int start_x;
  int start_y;
  pthread_mutex_t* hitbox_mutex;
};

void * thread_hash(void * args){
  pthread_detach(pthread_self());

  struct search_thread_params * thread_params = args;
  uint64_t** original_hashed_image = thread_params -> original_hashed_image;
  struct pixel** my_search_image = thread_params -> my_search_image;
  int** hitbox = thread_params -> hitbox;
  int original_dim_x = thread_params -> original_dim_x;
  int original_dim_y = thread_params -> original_dim_y;
  int start_x = thread_params -> start_x;
  int start_y = thread_params -> start_y;
  pthread_mutex_t* hitbox_mutex = thread_params -> hitbox_mutex;
  free(thread_params);

  

  uint64_t my_hash = hash8_gray_pixels(my_search_image,start_x,start_y);
  //printf("My%" PRIu64 "\n", my_hash);
  return NULL;
}

void split_hash(struct board** search_image,uint64_t** original_hashed_image, int original_dim_x, int original_dim_y)
{
  int dim_x = (*search_image) -> resolution_x;
  int dim_y = (*search_image) -> resolution_y;
  int new_dim_x = dim_x + (HASH_SIZE_X - (dim_x % HASH_SIZE_X));
  int new_dim_y = dim_y + (HASH_SIZE_Y - (dim_y % HASH_SIZE_Y));

  pthread_mutex_t* hitbox_mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(hitbox_mutex, NULL);

  //allocate the hit box
  int ** hitbox = calloc(original_dim_y,sizeof(int*));
  if(hitbox == NULL) fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
  for(int c1 = 0; c1 < original_dim_y; c1++){
    hitbox[c1] = calloc(original_dim_x, sizeof(int));
    if(hitbox[c1] == NULL) fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
  }

  resize_dimension(search_image,new_dim_x,new_dim_y);

  for(int c1 = 0; c1 < new_dim_y; c1 += HASH_SIZE_Y){
    for(int c2 = 0; c2 < new_dim_x; c2 += HASH_SIZE_X){
      printf("start_x: %d, start:y %d\n", c2, c1);
      struct search_thread_params * thread_params = malloc(sizeof(struct search_thread_params));
      thread_params -> original_hashed_image = original_hashed_image;
      thread_params -> my_search_image = (*search_image)->image;
      thread_params -> hitbox = hitbox;
      thread_params -> original_dim_x = original_dim_x;
      thread_params -> original_dim_y = original_dim_y;
      thread_params -> start_x = c2;
      thread_params -> start_y = c1;
      thread_params -> hitbox_mutex = hitbox_mutex;

      thread_hash(thread_params);
    }
  }
  printf("MAX BOARD.dim_x: %d, x: %d, y: %d\n", dim_x, new_dim_x, new_dim_y);

  for(int c1 = 0; c1 < original_dim_y; c1++)
  {
    free(hitbox[c1]);
  }
  free(hitbox);
  free(hitbox_mutex);

}

uint64_t** hash_original(struct board** original_image,int* original_dim_x, int* original_dim_y)
{
  *original_dim_x = (*original_image)->resolution_x-HASH_SIZE_X;
  *original_dim_y = (*original_image)->resolution_y-HASH_SIZE_Y;

  uint64_t** answer = malloc(sizeof(uint64_t*)*(*original_dim_y));

  struct pixel** img  = (*original_image)->image;

  for(int y = 0; y < *original_dim_y; ++y)
  {
    answer[y] = malloc(sizeof(uint64_t)*(*original_dim_x));
    for(int x = 0; x < *original_dim_x; ++x)
    {
      answer[y][x]=hash8_gray_pixels(img,x,y);
    }
  }
}

/*
  Designed to be ran on a grayscale 9x8 image
  creates a 64bit hash from an 9x8 board
  bit position refers to if pixel was brighter than avg
*/
uint64_t hash8_gray_board(struct board** board)
{
  uint64_t hash_val;
  int mask;
  int nth = 0;
  struct pixel pixel1;
  struct pixel pixel2;
  for(int y = 0; y < 8; ++y)
  {
      for(int x =0; x < 8; ++x)
      {
        int x2 = x+1;
        pixel1 = get_pixel(*board,&x,&y);
        pixel2 = get_pixel(*board,&x2,&y);

        if(pixel1.red <pixel2.red)
        {
          hash_val = (hash_val & ~(1<<nth)) | (1<<nth);
        }
        else
        {
          hash_val = (hash_val & ~(1<<nth)) | (0<<nth);
        }
        ++nth;
      }
  }
  return hash_val;
}

uint64_t hash8_gray_pixels(struct pixel** board, int start_x, int start_y)
{
  uint64_t hash_val;
  int mask;
  int nth = 0;
  struct pixel pixel1;
  struct pixel pixel2;
  for(int y = 0; y < 8; ++y)
  {
      for(int x =0; x < 8; ++x)
      {
        int x2 = x+1;
        pixel1 = board[y+start_y][x+start_x];
        pixel2 = board[y+start_y][x2+start_x];

        if(pixel1.red <pixel2.red)
        {
          hash_val = (hash_val & ~(1<<nth)) | (1<<nth);
        }
        else
        {
          hash_val = (hash_val & ~(1<<nth)) | (0<<nth);
        }
        ++nth;
      }
  }
  return hash_val;
}

int hamming_distance(uint64_t* hash1, uint64_t* hash2)
{
  uint64_t xor = (*hash1)^(*hash2);
  int diff = 0;
  for(; xor > 0; xor >>=1)
  {
    diff +=xor&1;
  }
  return diff;
}







/* WIP

uint64_t hash8_gray(struct board** board, int color_avg)
{
  uint64_t hash_val;
  int mask;
  int nth = 0;
  struct pixel pixel1;
  struct pixel pixel2;
  for(int y = 0; y < 16; ++y)
  {
      for(int x =0; x < 16; ++x)
      {
        int x2 = x+1;
        pixel1 = get_pixel(*board,&x,&y);
        pixel2 = get_pixel(*board,&x2,&y);

        if((pixel1.red>=color_avg && pixel2.red<color_avg) ||(pixel2.red>=color_avg && pixel1.red<color_avg))
        {
          hash_val = hash_val & ~(1<<nth) | (1<<nth);
        }
        else
        {
          hash_val = hash_val & ~(1<<nth) | (0<<nth);
        }
        ++nth;
      }
  }
  printf("nth: %d\n",nth);
  return hash_val;
}
*/
