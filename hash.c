#include "hash.h"
#include "image.h"
#include "hsv.h"
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

#define MAX_HAMMING 5
#define HASH_SIZE_X 9
#define HASH_SIZE_Y 8

#define HASH_SIZE 8

struct search_thread_params_gray
{
	uint64_t **original_hashed_image;
	struct pixel **my_search_image;
	int **hitbox;
	int original_dim_x;
	int original_dim_y;
	int start_x;
	int start_y;
	pthread_mutex_t *hitbox_mutex;
};

struct search_thread_params_color
{
	struct color_hash **original_hashed_image;
	struct pixel **my_search_image;
	int **hitbox;
	int original_dim_x;
	int original_dim_y;
	int start_x;
	int start_y;
	pthread_mutex_t *hitbox_mutex;
};

int hamming_distance(uint64_t *hash1, uint64_t *hash2)
{
	uint64_t xor = (*hash1) ^ (*hash2);
	int diff = 0;
	for (; xor > 0; xor >>= 1)
	{
		diff += xor&1;
	}
	return diff;
}

void *thread_hash_gray(void *args)
{

	struct search_thread_params_gray *thread_params = args;
	uint64_t **original_hashed_image = thread_params->original_hashed_image;
	struct pixel **my_search_image = thread_params->my_search_image;
	int **hitbox = thread_params->hitbox;
	int original_dim_x = thread_params->original_dim_x;
	int original_dim_y = thread_params->original_dim_y;
	int start_x = thread_params->start_x;
	int start_y = thread_params->start_y;
	pthread_mutex_t *hitbox_mutex = thread_params->hitbox_mutex;
	free(thread_params);

	uint64_t my_hash = hash8_gray_pixels(my_search_image, start_x, start_y);
	struct pixel corner = my_search_image[0][0];
	if (my_hash == 0) // && corner.red == 255 && corner.blue == 255 && corner.green == 255)
	{
		return NULL;
	}
	//printf("My%" PRIu64 "\n", my_hash);

	//search the hashed original image
	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			//if the hash matches, mark the hitbox
			int ham = hamming_distance(&my_hash, &original_hashed_image[y][x]);
			if (ham <= MAX_HAMMING)
			{
				pthread_mutex_lock(hitbox_mutex);
				hitbox[y][x]++;
				pthread_mutex_unlock(hitbox_mutex);
			}
		}
	}

	/*
      VALGRIND DOESNT LIKE THIS
       ||
      _||_
      \  /
       \/            */
	//pthread_exit(NULL);
	return NULL;
}
void *thread_hash_color(void *args)
{

	struct search_thread_params_color *thread_params = args;
	struct color_hash **original_hashed_image = thread_params->original_hashed_image;
	struct pixel **my_search_image = thread_params->my_search_image;
	int **hitbox = thread_params->hitbox;
	int original_dim_x = thread_params->original_dim_x;
	int original_dim_y = thread_params->original_dim_y;
	int start_x = thread_params->start_x;
	int start_y = thread_params->start_y;
	pthread_mutex_t *hitbox_mutex = thread_params->hitbox_mutex;
	free(thread_params);

	struct color_hash my_hash = hash8_color_pixels(my_search_image, start_x, start_y);
	//struct pixel corner = my_search_image[0][0];

	/*if (my_hash == 0) // && corner.red == 255 && corner.blue == 255 && corner.green == 255)
	{
		return NULL;
	}*/
	//printf("My%" PRIu64 "\n", my_hash);

	//search the hashed original image
	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			//if the hash matches, mark the hitbox
			int ham_red = hamming_distance(&my_hash.red, &original_hashed_image[y][x].red);
			int ham_green = hamming_distance(&my_hash.green, &original_hashed_image[y][x].green);
			int ham_blue = hamming_distance(&my_hash.blue, &original_hashed_image[y][x].blue);
			if (ham_red <= 10 && ham_green <= 10 && ham_blue <= 10)
			{
				pthread_mutex_lock(hitbox_mutex);
				hitbox[y][x]++;
				pthread_mutex_unlock(hitbox_mutex);
			}
		}
	}

	/*
      VALGRIND DOESNT LIKE THIS
       ||
      _||_
      \  /
       \/            */
	//pthread_exit(NULL);
	return NULL;
}

void split_hash_gray(struct board **search_image, uint64_t **original_hashed_image, int original_dim_x, int original_dim_y)
{
	int dim_x = (*search_image)->resolution_x;
	int dim_y = (*search_image)->resolution_y;
	int new_dim_x = dim_x;
	int new_dim_y = dim_y;
	if (dim_x % HASH_SIZE_X > 0)
		new_dim_x += (HASH_SIZE_X - (dim_x % HASH_SIZE_X));
	if (dim_y % HASH_SIZE_Y > 0)
		new_dim_y += (HASH_SIZE_Y - (dim_y % HASH_SIZE_Y));

	pthread_mutex_t *hitbox_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(hitbox_mutex, NULL);

	//allocate the hit box
	int **hitbox = calloc(original_dim_y, sizeof(int *));
	if (hitbox == NULL)
		fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		hitbox[c1] = calloc(original_dim_x, sizeof(int));
		if (hitbox[c1] == NULL)
			fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	}
	resize_dimension(search_image, new_dim_x, new_dim_y);
	int number_of_threads = new_dim_x * new_dim_y / (HASH_SIZE_X * HASH_SIZE_Y);
	pthread_t *children = malloc(sizeof(pthread_t) * number_of_threads);
	int counter = 0;
	printf("Threads: %d\n",number_of_threads);
	for (int c1 = 0; c1 < new_dim_y; c1 += HASH_SIZE_Y)
	{
		for (int c2 = 0; c2 < new_dim_x; c2 += HASH_SIZE_X)
		{
			struct search_thread_params_gray *thread_params = malloc(sizeof(struct search_thread_params_gray));
			thread_params->original_hashed_image = original_hashed_image;
			thread_params->my_search_image = (*search_image)->image;
			thread_params->hitbox = hitbox;
			thread_params->original_dim_x = original_dim_x;
			thread_params->original_dim_y = original_dim_y;
			thread_params->start_x = c2;
			thread_params->start_y = c1;
			thread_params->hitbox_mutex = hitbox_mutex;
			pthread_create(&children[counter], NULL, thread_hash_gray, thread_params); // thread_hash(thread_params);
			counter += 1;
		}
	}

	for (int c1 = 0; c1 < number_of_threads; c1++)
	{
		pthread_join(children[c1], NULL);
	}

	/////SAVE THE HITBOX TO AN IMAGE
	struct board *visualizaiton = make_board(&original_dim_x, &original_dim_y);

	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			set_pixel(visualizaiton, &x, &y, hitbox[y][x], hitbox[y][x], hitbox[y][x]);
		}
	}
	save_ppm(visualizaiton, "visual_gray.ppm");
	free_board(&visualizaiton);
	////////////////////////////////////

	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		free(hitbox[c1]);
	}
	free(hitbox);
	free(hitbox_mutex);
	free(children);
}

void split_hash_color(struct board **search_image, struct color_hash** original_hashed_image, int original_dim_x, int original_dim_y)
{
	int dim_x = (*search_image)->resolution_x;
	int dim_y = (*search_image)->resolution_y;
	int new_dim_x = dim_x;
	int new_dim_y = dim_y;
	if (dim_x % HASH_SIZE_X > 0)
		new_dim_x += (HASH_SIZE_X - (dim_x % HASH_SIZE_X));
	if (dim_y % HASH_SIZE_Y > 0)
		new_dim_y += (HASH_SIZE_Y - (dim_y % HASH_SIZE_Y));

	pthread_mutex_t *hitbox_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(hitbox_mutex, NULL);

	//allocate the hit box
	int **hitbox = calloc(original_dim_y, sizeof(int *));
	if (hitbox == NULL)
		fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		hitbox[c1] = calloc(original_dim_x, sizeof(int));
		if (hitbox[c1] == NULL)
			fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	}
	resize_dimension(search_image, new_dim_x, new_dim_y);
	int number_of_threads = new_dim_x * new_dim_y / (HASH_SIZE_X * HASH_SIZE_Y);
	pthread_t *children = malloc(sizeof(pthread_t) * number_of_threads);
	int counter = 0;
	for (int c1 = 0; c1 < new_dim_y; c1 += HASH_SIZE_Y)
	{
		for (int c2 = 0; c2 < new_dim_x; c2 += HASH_SIZE_Y)
		{
			struct search_thread_params_color *thread_params = malloc(sizeof(struct search_thread_params_color));
			thread_params->original_hashed_image = original_hashed_image;
			thread_params->my_search_image = (*search_image)->image;
			thread_params->hitbox = hitbox;
			thread_params->original_dim_x = original_dim_x;
			thread_params->original_dim_y = original_dim_y;
			thread_params->start_x = c2;
			thread_params->start_y = c1;
			thread_params->hitbox_mutex = hitbox_mutex;
			pthread_create(&children[counter], NULL, thread_hash_color, thread_params); // thread_hash(thread_params);
			counter += 1;
		}
	}

	for (int c1 = 0; c1 < number_of_threads; c1++)
	{
		pthread_join(children[c1], NULL);
	}

	/////SAVE THE HITBOX TO AN IMAGE
	struct board *visualizaiton = make_board(&original_dim_x, &original_dim_y);

	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			set_pixel(visualizaiton, &x, &y, hitbox[y][x], hitbox[y][x], hitbox[y][x]);
		}
	}
	save_ppm(visualizaiton, "visual_color.ppm");
	free_board(&visualizaiton);
	////////////////////////////////////

	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		free(hitbox[c1]);
	}
	free(hitbox);
	free(hitbox_mutex);
	free(children);
}

uint64_t **hash_original_gray(struct board **original_image, int *original_dim_x, int *original_dim_y)
{
	*original_dim_x = (*original_image)->resolution_x - HASH_SIZE_X;
	*original_dim_y = (*original_image)->resolution_y - HASH_SIZE_Y;

	uint64_t **answer = malloc(sizeof(uint64_t *) * (*original_dim_y));
	if (answer == NULL)
	{
		fprintf(stderr, "ERROR: Could not malloc in hash_original\n");
	}

	struct pixel **img = (*original_image)->image;

	for (int y = 0; y < *original_dim_y; ++y)
	{
		answer[y] = malloc(sizeof(uint64_t)*(*original_dim_x));
		if (answer[y] == NULL)
		{
			fprintf(stderr, "ERROR: Could not malloc in hash_original\n");
		}
		for (int x = 0; x < *original_dim_x; ++x)
		{
			answer[y][x] = hash8_gray_pixels(img, x, y);
		}
	}

	return answer;
}

struct color_hash** hash_original_color(struct board** original_image,int* original_dim_x, int* original_dim_y)
{
	*original_dim_x = (*original_image)->resolution_x - HASH_SIZE_X;
	*original_dim_y = (*original_image)->resolution_y - HASH_SIZE_Y;

	struct color_hash** answer = malloc(sizeof(struct color_hash*) * (*original_dim_y));
	if (answer == NULL)
	{
		fprintf(stderr, "ERROR: Could not malloc in hash_original\n");
	}

	struct pixel **img = (*original_image)->image;

	for (int y = 0; y < *original_dim_y; ++y)
	{
		answer[y] = malloc(sizeof(struct color_hash)*(*original_dim_x));
		if (answer[y] == NULL)
		{
			fprintf(stderr, "ERROR: Could not malloc in hash_original\n");
		}
		struct color_hash sh;
		for (int x = 0; x < *original_dim_x; ++x)
		{
			sh = hash8_color_pixels(img, x, y);
			answer[y][x].red = sh.red;
			answer[y][x].green = sh.green;
			answer[y][x].blue = sh.blue;
		}
	}

	return answer;
}

/*
  Designed to be ran on a grayscale 9x8 image
  creates a 64bit hash from an 9x8 board
  bit position refers to if pixel was brighter than avg
*/
uint64_t hash8_gray_board(struct board **board)
{
	uint64_t hash_val;
	int mask;
	int nth = 0;
	struct pixel pixel1;
	struct pixel pixel2;
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			int x2 = x + 1;
			pixel1 = get_pixel(*board, &x, &y);
			pixel2 = get_pixel(*board, &x2, &y);

			if (pixel1.red < pixel2.red)
			{
				hash_val = (hash_val & ~(1 << nth)) | (1 << nth);
			}
			else
			{
				hash_val = (hash_val & ~(1 << nth)) | (0 << nth);
			}
			++nth;
		}
	}
	return hash_val;
}

uint64_t hash8_gray_pixels(struct pixel **board, int start_x, int start_y)
{
	uint64_t hash_val;
	int mask;
	int nth = 0;
	struct pixel pixel1;
	struct pixel pixel2;
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			int x2 = x + 1;
			pixel1 = board[y + start_y][x + start_x];
			pixel2 = board[y + start_y][x2 + start_x];

			if (pixel1.red < pixel2.red)
			{
				hash_val = (hash_val & ~(1 << nth)) | (1 << nth);
			}
			else
			{
				hash_val = (hash_val & ~(1 << nth)) | (0 << nth);
			}
			++nth;
		}
	}
	return hash_val;
}



struct color_hash hash8_color_pixels(struct pixel **board, int start_x, int start_y)
{
	struct color_hash answer;

	int nth = 0;
	struct pixel pixel1;
	struct pixel pixel2;
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			int x2 = x + 1;
			pixel1 = board[y + start_y][x + start_x];
			pixel2 = board[y + start_y][x2 + start_x];

			//hash red pixels
			if (pixel1.red < pixel2.red)
			{
				answer.red = (answer.red & ~(1 << nth)) | (1 << nth);
			}
			else
			{
				answer.red = (answer.red & ~(1 << nth)) | (0 << nth);
			}
			//hash green pixels
			if(pixel1.green <pixel2.green)
        {
          answer.green = (answer.green & ~(1<<nth)) | (1<<nth);
        }
        else
        {
          answer.green = (answer.green & ~(1<<nth)) | (0<<nth);
        }
			//hash blue pixels
			if(pixel1.blue <pixel2.blue)
        {
          answer.blue = (answer.blue & ~(1<<nth)) | (1<<nth);
        }
        else
        {
          answer.blue = (answer.blue & ~(1<<nth)) | (0<<nth);
        }

			++nth;
		}
	}
	return answer;
}


/* ================== HSV HASH ================== */
struct search_thread_params_HSV
{
	struct hsv_hash **original_hashed_image;
	struct pixel **my_search_image;
	int** hitbox;
	int original_dim_x;
	int original_dim_y;
	int start_x;
	int start_y;
	pthread_mutex_t *hitbox_mutex;
};

struct hsv_hash hash8_hsv_pixels(struct pixel** board,int start_x, int start_y){
	struct hsv_hash answer;

	int nth = 0;
	struct hsv hsv1;
	struct hsv hsv2;
	struct pixel pixel;
	int total_hue = 0;
	double hash2 = 0.0;
	for(int y = 0; y < 8; ++y){
		for(int x = 0; x < 8; ++x){
			int x2 = x + 1;
			pixel = board[y + start_y][x + start_x];
			hsv1 = RGBtoHSV(pixel.red,pixel.green,pixel.blue);
			total_hue += hsv1.h;
			pixel = board[y + start_y][x2 + start_x];
			hsv2 = RGBtoHSV(pixel.red,pixel.green,pixel.blue);

			hash2 += hsv1.h/360;// * (y * 8 + x2);
			//hue
			if(hsv1.h < hsv2.h) answer.h = (answer.h & ~(1 << nth)) | (1 << nth);
			else answer.h = (answer.h & ~(1 << nth)) | (0 << nth);
			//saturation
			if(hsv1.s < hsv2.s) answer.s = (answer.s & ~(1 << nth)) | (1 << nth);
			else answer.s = (answer.s & ~(1 << nth)) | (0 << nth);
			//value
			if(hsv1.v < hsv2.v) answer.v = (answer.v & ~(1 << nth)) | (1 << nth);
			else answer.v = (answer.v & ~(1 << nth)) | (0 << nth);
			++nth;
		}

	}
	pixel = board[start_y][start_x];
	answer.corner_hue = RGBtoHSV(pixel.red, pixel.green, pixel.blue).h;
	answer.avg_hue = total_hue/64;
	answer.hash2 = hash2;
	return answer;
}

struct hsv_hash** hash_original_HSV(struct board** original_image,int* original_dim_x, int* original_dim_y){

	*original_dim_x = (*original_image)->resolution_x - HASH_SIZE_X;
	*original_dim_y = (*original_image)->resolution_y - HASH_SIZE_Y;

	struct hsv_hash** answer = malloc(sizeof(struct hsv_hash*) * (*original_dim_y));
	if(answer == NULL) fprintf(stderr, "ERROR: Could not malloc in hash_original\n");

	struct pixel ** img = (*original_image) -> image;

	for(int y = 0; y < *original_dim_y; ++y){
		answer[y] = malloc(sizeof(struct hsv_hash)*(*original_dim_x));
		if(answer[y] == NULL) fprintf(stderr, "ERROR: Could not malloc in hash_original\n");

		struct hsv_hash sh;
		for (int x = 0; x < *original_dim_x; ++x)
		{
			sh = hash8_hsv_pixels(img, x, y);
			answer[y][x].h = sh.h;
			answer[y][x].s = sh.s;
			answer[y][x].v = sh.v;
		}
	}
	return answer;
}

void split_hash_HSV(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y){
	int dim_x = (*search_image)->resolution_x;
	int dim_y = (*search_image)->resolution_y;
	int new_dim_x = dim_x;
	int new_dim_y = dim_y;
	if (dim_x % HASH_SIZE > 0)
		new_dim_x += (HASH_SIZE - (dim_x % HASH_SIZE)) + 1;
	if (dim_y % HASH_SIZE > 0)
		new_dim_y += (HASH_SIZE - (dim_y % HASH_SIZE));

	printf("Search new dim size-- x: %d, y: %d\n", new_dim_x, new_dim_y);

	pthread_mutex_t *hitbox_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(hitbox_mutex, NULL);

	//allocate the hit box
	int **hitbox = calloc(original_dim_y, sizeof(int *));
	if (hitbox == NULL)
		fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		hitbox[c1] = calloc(original_dim_x, sizeof(int));
		if (hitbox[c1] == NULL)
			fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	}

	resize_dimension(search_image, new_dim_x, new_dim_y);

	int number_of_threads = (new_dim_x-1) * new_dim_y / (HASH_SIZE * HASH_SIZE);
	pthread_t *children = malloc(sizeof(pthread_t) * number_of_threads);
	int counter = 0;
	printf("Threads: %d\n",number_of_threads);

	for (int c1 = 0; c1 < new_dim_y; c1 += HASH_SIZE)
	{
		for (int c2 = 0; c2 < new_dim_x-1; c2 += HASH_SIZE)
		{
			struct search_thread_params_HSV *thread_params = malloc(sizeof(struct search_thread_params_HSV));
			thread_params->original_hashed_image = original_hashed_image;
			thread_params->my_search_image = (*search_image)->image;
			thread_params->hitbox = hitbox;
			thread_params->original_dim_x = original_dim_x;
			thread_params->original_dim_y = original_dim_y;
			thread_params->start_x = c2;
			thread_params->start_y = c1;
			thread_params->hitbox_mutex = hitbox_mutex;
			pthread_create(&children[counter], NULL, thread_hash_HSV, thread_params); // thread_hash(thread_params);
			counter += 1;
		}
	}
	printf("# threads created: %d\n", counter);
	int num_threads = 0;
	for (int c1 = 0; c1 < number_of_threads; c1++)
	{
		pthread_join(children[c1], NULL);
		num_threads += 1;
	}
	printf("Threads joined: %d\n", num_threads);

	/////SAVE THE HITBOX TO AN IMAGE
	int new_size_x = original_dim_x + HASH_SIZE;
	int new_size_y = original_dim_y + HASH_SIZE;
	struct board *visualizaiton = make_board(&new_size_x, &new_size_y);

	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			for(int c1 = 0; c1 < HASH_SIZE; c1++){
				for(int c2 = 0; c2 < HASH_SIZE; c2++){
					int del_x = x+c2;
					int del_y = y+c1;
					if(hitbox[y][x] != 0)
						set_pixel(visualizaiton, &del_x, &del_y, hitbox[y][x], hitbox[y][x], hitbox[y][x]);
				}
			}
		}
	}
	save_ppm(visualizaiton, "visual_hsv.ppm");
	free_board(&visualizaiton);
	////////////////////////////////////

	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		free(hitbox[c1]);
	}
	free(hitbox);
	free(hitbox_mutex);
	free(children);

}

void * thread_hash_HSV(void * args){
	struct search_thread_params_HSV *thread_params = args;
	struct hsv_hash **original_hashed_image = thread_params->original_hashed_image;
	struct pixel **my_search_image = thread_params->my_search_image;
	int **hitbox = thread_params->hitbox;
	int original_dim_x = thread_params->original_dim_x;
	int original_dim_y = thread_params->original_dim_y;
	int start_x = thread_params->start_x;
	int start_y = thread_params->start_y;
	pthread_mutex_t *hitbox_mutex = thread_params->hitbox_mutex;
	free(thread_params);

	struct hsv_hash my_hash = hash8_hsv_pixels(my_search_image, start_x, start_y);
	double my_avg_hue = my_hash.avg_hue;
	double my_corner_hue = my_hash.corner_hue;
	double my_hash2 = my_hash.hash2;
	struct pixel corner = my_search_image[0][0];
	// if(my_hash.h == 0 && corner.red >= 255 && corner.green >= 255 && corner.blue >= 255){
	// 	return NULL;
	// }

	int scale_h = 4;
	int scale_s = 1;
	int scale_v = 1;

	double weight = 1.0 / (scale_h + scale_s + scale_v);

	double weight_h = weight * scale_h;
	double weight_s = weight * scale_s;
	double weight_v = weight * scale_v;

	double diff = 65;
	int best_x = -1;
	int best_y = -1;

	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			//Idea for better hits, limit the h range
			//if the hash matches, mark the hitbox
			int ham_h = hamming_distance(&my_hash.h, &original_hashed_image[y][x].h);
			int ham_s = hamming_distance(&my_hash.s, &original_hashed_image[y][x].s);
			int ham_v = hamming_distance(&my_hash.v, &original_hashed_image[y][x].v);
			double check = (weight_h * ham_h) + (weight_s * ham_s) + (weight_v * ham_v);
			if (check < diff
					//&& fabs(my_corner_hue - original_hashed_image[y][x].corner_hue) < 20.0
					&& fabs(my_avg_hue - original_hashed_image[y][x].avg_hue) < 20.0
					&& fabs(my_hash2 - original_hashed_image[y][x].hash2) < 20.0
				)
			{
				best_x=x;
				best_y=y;
				diff = check;
			}
		}
	}
	if(diff < 20){
		pthread_mutex_lock(hitbox_mutex);
		hitbox[best_y][best_x]=255;         //WRONG////////////////////////////////////
		pthread_mutex_unlock(hitbox_mutex);
	}
	else{
		//printf("Diff too high %f\n", diff);
	}
	pthread_exit(NULL);
}
