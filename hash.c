#include "hash.h"
#include "image.h"
#include "hsv.h"
#include "score.h"
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

/* ================== HSV HASH ================== */
struct search_thread_params_HSV
{
	struct hsv_hash **original_hashed_image;
	struct pixel **my_search_image;
	int** hitbox;
	int original_dim_x;
	int original_dim_y;
	int search_dim_x;
	int serach_dim_y;
	int total_threads;
	int start_x;
	int start_y;
	pthread_mutex_t *hitbox_mutex;
};
struct best_score_info find_image(struct board** original_image, struct board** search_image,double scale)
{
	int original_dim_x=-1;
  int original_dim_y=-1;

	resize_percent(search_image, scale);

  struct hsv_hash** hashed_original = hash_original_HSV(original_image,&original_dim_x,&original_dim_y);
  struct best_score_info result = hash_thread_allocator(search_image,hashed_original, original_dim_x, original_dim_y);

	for(int y = 0; y < original_dim_y; ++y)
  {
   free(hashed_original[y]);
  }
  free(hashed_original);

	return result;
}
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

struct hsv_hash** hash_original_HSV(struct board** original_image, int* original_dim_x, int* original_dim_y){

	*original_dim_x = (*original_image)->resolution_x;
	*original_dim_y = (*original_image)->resolution_y;

	int hash_board_x = (*original_image)->resolution_x - HASH_SIZE;
	int hash_board_y = (*original_image)->resolution_y - HASH_SIZE;

	struct hsv_hash** answer = calloc((*original_dim_y), sizeof(struct hsv_hash*));
	if(answer == NULL) fprintf(stderr, "ERROR: Could not malloc in hash_original\n");

	struct pixel ** img = (*original_image) -> image;

	for(int y = 0; y < hash_board_y; ++y){
		answer[y] = calloc((*original_dim_x), sizeof(struct hsv_hash));
		if(answer[y] == NULL) fprintf(stderr, "ERROR: Could not malloc in hash_original\n");

		struct hsv_hash sh;
		for (int x = 0; x < hash_board_x; ++x)
		{
			sh = hash8_hsv_pixels(img, x, y);
			answer[y][x].h = sh.h;
			answer[y][x].s = sh.s;
			answer[y][x].v = sh.v;
			answer[y][x].avg_hue = sh.avg_hue;
			answer[y][x].corner_hue = sh.corner_hue;
			answer[y][x].hash2 = sh.hash2;
		}
	}
	//fill out the rest of the hash_board
	for(int c1 = hash_board_y; c1 < *original_dim_y; ++c1){
		answer[c1] = calloc((*original_dim_x), sizeof(struct hsv_hash));
	}
	return answer;
}

struct best_score_info hash_thread_allocator(struct board **search_image, struct hsv_hash **original_hashed_image, int original_dim_x, int original_dim_y)
{
	/* resize the search image */
	int search_dim_x = (*search_image)->resolution_x;
	int search_dim_y = (*search_image)->resolution_y;
	int new_search_dim_x = search_dim_x;
	int new_search_dim_y = search_dim_y;
	if (search_dim_x % HASH_SIZE > 0)
		new_search_dim_x += (HASH_SIZE - (new_search_dim_x % HASH_SIZE));
	if (search_dim_y % HASH_SIZE > 0)
		new_search_dim_y += (HASH_SIZE - (new_search_dim_y % HASH_SIZE));
	new_search_dim_x += 1;
	//resize the search image
	resize_dimension(search_image, new_search_dim_x, new_search_dim_y);

	printf("Search new dim size-- x: %d, y: %d\n", new_search_dim_x, new_search_dim_y);

	pthread_mutex_t *hitbox_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(hitbox_mutex, NULL);


	//allocate the hit box (SAME SIZE AS ORIGINAL IMAGE)
	int **hitbox = calloc(original_dim_y, sizeof(int *));
	if (hitbox == NULL)
		fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		hitbox[c1] = calloc(original_dim_x, sizeof(int));
		if (hitbox[c1] == NULL)
			fprintf(stderr, "ERROR: Failed to malloc (hitbox)");
	}


	int max_possible_threads = (new_search_dim_x-1) * new_search_dim_y / (HASH_SIZE * HASH_SIZE);
	if(max_possible_threads>63)
	{
		max_possible_threads=63;
	}
	pthread_t *children = malloc(sizeof(pthread_t) * max_possible_threads);

	//because this function will do work starting at 0 0
	int start_x = 8;
	int start_y = 0;

	printf("there will be %d threads created\n",max_possible_threads);

	for(int thread = 0; thread < max_possible_threads; ++thread)
	{
		struct search_thread_params_HSV *thread_params = malloc(sizeof(struct search_thread_params_HSV));
		thread_params->original_hashed_image = original_hashed_image;
		thread_params->my_search_image = (*search_image)->image;
		thread_params->hitbox = hitbox;
		thread_params->original_dim_x = original_dim_x;
		thread_params->original_dim_y = original_dim_y;
		thread_params->search_dim_x = new_search_dim_x;
		thread_params->serach_dim_y = new_search_dim_y;
		thread_params->total_threads = max_possible_threads;
		thread_params->start_x = start_x;
		thread_params->start_y = start_y;
		thread_params->hitbox_mutex = hitbox_mutex;
		pthread_create(&children[thread], NULL, call_thread, thread_params);
		// printf("created thread at (%d,%d)\n",start_x,start_y);
		start_x+=8;
		if(start_x>new_search_dim_x-HASH_SIZE-1)
		{
			start_x-=new_search_dim_x-HASH_SIZE-1;
			start_y+=8;
			//probably dont need this
			/*if(start_y>new_dim_y)
			{
				break;
			}*/
		}
	}


	hash_worker(original_hashed_image, (*search_image)->image, hitbox, original_dim_x,
									original_dim_y, new_search_dim_x, new_search_dim_y, start_x, start_y, hitbox_mutex, max_possible_threads);


	int num_threads = 0;
	for (int c1 = 0; c1 < max_possible_threads; c1++)
	{
		if(0!=pthread_join(children[c1], NULL))
		{
			fprintf(stderr,"ERROR\n");
		}
		num_threads+=1;
	}
	printf("Threads joined: %d\n", num_threads);

	//calculate the score:

	struct best_score_info best_score = calc_best_score(hitbox, original_dim_x, original_dim_y, new_search_dim_x, new_search_dim_y);

	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		free(hitbox[c1]);
	}
	free(hitbox);
	free(hitbox_mutex);
	free(children);


	return best_score;
	/* VISUALIZATION CODE
	/////SAVE THE HITBOX TO AN IMAGE
	struct board *visualization = make_board(&original_dim_x, &original_dim_y);
	double score = best_score.score;
	int search_start_x = best_score.search_start_x;
	int search_start_y = best_score.search_start_y;
	double extra_info = best_score.extra_info;
	int total_hits = best_score.total_hits;

	printf("Score: %f\nStart x: %d, y: %d\nDim x: %d, y: %d\nExtra Info: %f\nTotal Hits:%d\n",
					score, search_start_x, search_start_y, new_search_dim_x, new_search_dim_y, extra_info, total_hits);

	//modify visualization to show the best score location
	if(score > -1){
		for(int row = 0; row < new_search_dim_y; row++){
			for(int col = 0; col < new_search_dim_x; col++){
				int del_x = min_int(col + search_start_x,original_dim_x-1);
				int del_y = min_int(row + search_start_y,original_dim_y-1);
				set_pixel(visualization, &del_x, &del_y, 255, 0, 0);
			}
		}
	}
	else{
		printf("Did not find the image! \n");
	}

	for (int y = 0; y < original_dim_y; ++y)
	{
		for (int x = 0; x < original_dim_x; ++x)
		{
			for(int c1 = 0; c1 < HASH_SIZE; c1++){
				for(int c2 = 0; c2 < HASH_SIZE; c2++){
					int del_x = min_int(x+c2,original_dim_x);
					int del_y = min_int(y+c1, original_dim_y);
					if(hitbox[y][x] != 0)
						set_pixel(visualization, &del_x, &del_y, hitbox[y][x], hitbox[y][x], hitbox[y][x]);
				}
			}
		}
	}

	save_ppm(visualization, "visual_hsv.ppm");
	free_board(&visualization);
	////////////////////////////////////
	*/
}

void hash_worker(struct hsv_hash **original_hashed_image, struct pixel **my_search_image,
								int **hitbox, int original_dim_x, int original_dim_y,
								int search_dim_x, int search_dim_y, int start_x, int start_y,
								pthread_mutex_t* hitbox_mutex, int total_threads)
{
	int scale_h = 4;
	int scale_s = 1;
	int scale_v = 1;


	double best_weighted = 65;
	double best_average = 100000;
	double best_total = 100000;
	int best_x = -1;
	int best_y = -1;

	int x = start_x;
	int y = start_y;

	while(y < search_dim_y)
	{
		struct hsv_hash my_hash = hash8_hsv_pixels(my_search_image, x, y);
		double my_avg_hue = my_hash.avg_hue;
		double my_corner_hue = my_hash.corner_hue;
		double my_hash2 = my_hash.hash2;
		struct pixel corner = my_search_image[y][x];
		if(my_hash.h == 0 && corner.red >= 255 && corner.green >= 255 && corner.blue >= 255){
			continue;
		}
		double weight = 1.0 / (scale_h + scale_s + scale_v);

		double weight_h = weight * scale_h;
		double weight_s = weight * scale_s;
		double weight_v = weight * scale_v;

		int original_hash_x = original_dim_x - HASH_SIZE;
		int original_hash_y = original_dim_y - HASH_SIZE;

		for (int y2 = 0; y2 < original_hash_y; ++y2)
		{
			for (int x2 = 0; x2 < original_hash_x; ++x2)
			{
				//Idea for better hits, limit the h range
				//if the hash matches, mark the hitbox
				int ham_h = hamming_distance(&my_hash.h, &original_hashed_image[y2][x2].h);
				int ham_s = hamming_distance(&my_hash.s, &original_hashed_image[y2][x2].s);
				int ham_v = hamming_distance(&my_hash.v, &original_hashed_image[y2][x2].v);
				double check_weight = (weight_h * ham_h) + (weight_s * ham_s) + (weight_v * ham_v);
				double check_average = fabs(my_avg_hue - original_hashed_image[y2][x2].avg_hue);
				double check_total = fabs(my_hash2 - original_hashed_image[y2][x2].hash2);
				if (check_weight < best_weighted
						&& fabs(my_corner_hue - original_hashed_image[y2][x2].corner_hue) < 20.0
						&&  check_average < 10.0
						&& 	check_total < 10.0
					)
				{
					best_x=x2;
					best_y=y2;
					best_weighted = check_weight;
					best_average = check_average;
					best_total = check_total;
				}
			}
		}
		if (best_weighted < 20)
		{
			pthread_mutex_lock(hitbox_mutex);
			hitbox[best_y][best_x] = 255; //WRONG////////////////////////////////////
			pthread_mutex_unlock(hitbox_mutex);
		}

		x+=(8*(total_threads+1));
		while(x>search_dim_x-HASH_SIZE-1)
		{
			x-=search_dim_x-HASH_SIZE-1;
			y+=8;
		}

		best_weighted = 65;
		best_average = 100000;
		best_total = 100000;
		best_x = -1;
		best_y = -1;
	}
}


void* call_thread(void* args)
{
	struct search_thread_params_HSV *thread_params = args;
	struct hsv_hash **original_hashed_image = thread_params->original_hashed_image;
	struct pixel **my_search_image = thread_params->my_search_image;
	int **hitbox = thread_params->hitbox;
	int original_dim_x = thread_params->original_dim_x;
	int original_dim_y = thread_params->original_dim_y;
	int search_dim_x = thread_params->search_dim_x;
	int search_dim_y = thread_params->serach_dim_y;
	int total_threads = thread_params->total_threads;
	int start_x = thread_params->start_x;
	int start_y = thread_params->start_y;
	pthread_mutex_t *hitbox_mutex = thread_params->hitbox_mutex;
	free(thread_params);


	hash_worker(original_hashed_image, my_search_image, hitbox, original_dim_x,
									original_dim_y, search_dim_x, search_dim_y, start_x, start_y, hitbox_mutex, total_threads);

	pthread_exit(NULL);

}
