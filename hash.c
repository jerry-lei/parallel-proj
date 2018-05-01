#include "hash.h"
#include "image.h"
#include "hsv.h"
#include "score.h"

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

#define NUMBER_THREADS 64

#define HASH_SIZE 8

/* Calculates the number of different bits between two hashed values
	-- Returns a number between 0-63.
*/
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

/* Struct used to pass in variables to thread function */
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
	pthread_mutex_t ***hitbox_mutex;
};

/* Main driver function to find images */
struct best_score_info find_image(struct board** original_image, struct board** search_image,double scale)
{
	int original_dim_x=-1;
  int original_dim_y=-1;

	/* Scale the search image by the specified percentage on the parameter */
	resize_percent(search_image, scale);

	/* Generate the board for the hashed original image -- this will be used for comparisons in threads */
  struct hsv_hash** hashed_original = hash_original_HSV(original_image,&original_dim_x,&original_dim_y);

	/* Call the function to do all of the computations for image recognition -- we pass in the hsv_hash board of original */
  struct best_score_info result = hash_thread_allocator(search_image,hashed_original, original_dim_x, original_dim_y);


	/* Free the HSV_Hash board */
	for(int y = 0; y < original_dim_y; ++y)
  {
  	free(hashed_original[y]);
  }
  free(hashed_original);

	/* Return the best score -- struct located in score.h */
	return result;
}

/* Function used to hash an 8x8 chunk of a board */
struct hsv_hash hash8_hsv_pixels(struct pixel** board,int start_x, int start_y){

	/* Return struct */
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
	struct hsv temp_hsv = RGBtoHSV(pixel.red, pixel.green, pixel.blue);
	/* Set the return values of hashes */
	answer.corner.h = temp_hsv.h;
	answer.corner.s = temp_hsv.s;
	answer.corner.v = temp_hsv.v;
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
			answer[y][x].corner = sh.corner;
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

	//pthread_mutex_init(hitbox_mutex, NULL);


	//allocate the hit box and mutex (SAME SIZE AS ORIGINAL IMAGE)
	pthread_mutex_t **hitbox_mutex = calloc(original_dim_y,sizeof(pthread_mutex_t*));
	int **hitbox = calloc(original_dim_y, sizeof(int *));

	if (hitbox == NULL || hitbox_mutex == NULL)
	{
		fprintf(stderr, "ERROR: Failed to malloc (hitbox)\n");
	}

	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		hitbox[c1] = calloc(original_dim_x, sizeof(int));
		hitbox_mutex[c1] = calloc(original_dim_x,sizeof(pthread_mutex_t));

		if (hitbox[c1] == NULL || hitbox_mutex[c1]==NULL)
		{
			fprintf(stderr, "ERROR: Failed to malloc (hitbox)\n");
		}
		for (int c2 = 0; c2 < original_dim_y; c2++)
		{
			if(pthread_mutex_init(&(hitbox_mutex[c1][c2]), NULL)!=0)
			{
				fprintf(stderr, "ERROR: Failed to initialize a mutex\n");
			}
		}
	}


	int max_possible_threads = (new_search_dim_x-1) * new_search_dim_y / (HASH_SIZE * HASH_SIZE);
	if(max_possible_threads> (NUMBER_THREADS - 1))
	{
		max_possible_threads=(NUMBER_THREADS - 1);
	}
	pthread_t *children = malloc(sizeof(pthread_t) * max_possible_threads);

	//because this function will do work starting at 0 0
	int start_x = 8;
	int start_y = 0;

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
		thread_params->hitbox_mutex = &hitbox_mutex;
		pthread_create(&children[thread], NULL, call_thread, thread_params);
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
									original_dim_y, new_search_dim_x, new_search_dim_y, start_x, start_y, &hitbox_mutex, max_possible_threads);


	int num_threads = 0;
	for (int c1 = 0; c1 < max_possible_threads; c1++)
	{
		if(0!=pthread_join(children[c1], NULL))
		{
			fprintf(stderr,"ERROR\n");
		}
		num_threads+=1;
	}

	//CALCULATE THE DISTRIBUTION
	double* optimal_distribution_x = calloc(NUMBER_BUCKETS,sizeof(double));
	double* optimal_distribution_y = calloc(NUMBER_BUCKETS,sizeof(double));
	////////////////////////////

	int width_x = ceil((double)new_search_dim_x/NUMBER_BUCKETS);
	int width_y = ceil((double)new_search_dim_y/NUMBER_BUCKETS);

	int total_passed_sky_filter = 0;

	for(int y_bucket_pos = 0; y_bucket_pos < new_search_dim_y; y_bucket_pos++)
	{
		for(int x_bucket_pos = 0; x_bucket_pos < new_search_dim_x; x_bucket_pos++)
		{
			struct pixel pixel = get_pixel(*search_image,&x_bucket_pos,&y_bucket_pos);
			struct hsv hsv = RGBtoHSV(pixel.red,pixel.green,pixel.blue);
			if(sky_filter(&hsv)==0)
			{
				optimal_distribution_x[x_bucket_pos/width_x]+=1;
				optimal_distribution_y[y_bucket_pos/width_y]+=1;
				total_passed_sky_filter += 1;
			}
		}
	}
	for(int c1 = 0;c1 < NUMBER_BUCKETS; c1++){
		optimal_distribution_x[c1] /= (total_passed_sky_filter);
		optimal_distribution_y[c1] /= (total_passed_sky_filter);
	}

	//calculate the score:
	struct best_score_info best_score = calc_best_score(hitbox, original_dim_x, original_dim_y, new_search_dim_x, new_search_dim_y,&optimal_distribution_x,&optimal_distribution_y);

	free(optimal_distribution_x);
	free(optimal_distribution_y);

	/////////////////TEMPORRARY HITBOX WRITER////////////////////
	// int new_size_x = original_dim_x;
	// int new_size_y = original_dim_y;
	// struct board* visualization = make_board(&new_size_x, &new_size_y);

	// for (int y = 0; y < original_dim_y-HASH_SIZE; ++y)
	// {
	// 	for (int x = 0; x < original_dim_x-HASH_SIZE; ++x)
	// 	{
	// 		for(int c1 = 0; c1 < HASH_SIZE; c1++){
	// 			for(int c2 = 0; c2 < HASH_SIZE; c2++){
	// 				int del_x = x+c2;
	// 				int del_y = y+c1;
	// 				if(hitbox[y][x] != 0)
	// 					set_pixel(visualization, &del_x, &del_y, 255, 255,255);
	// 			}
	// 		}
	// 	}
	// }
	// char* fname = malloc(sizeof(char)*100);
	// sprintf(fname,"hitboxes/hitbox_%dx%d.ppm",search_dim_x,search_dim_y);
	// save_ppm(visualization,fname);
	// free_board(&visualization);
	// free(fname);
	/////////////////////////////////////////////////////////////

	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		for (int c2 = 0; c2 < original_dim_y; c2++)
		{
			pthread_mutex_destroy(&(hitbox_mutex[c1][c2]));
		}
	}
	for (int c1 = 0; c1 < original_dim_y; c1++)
	{
		free(hitbox[c1]);
		free(hitbox_mutex[c1]);
	}
	free(hitbox);
	free(hitbox_mutex);
	free(children);


	return best_score;
}

void hash_worker(struct hsv_hash **original_hashed_image, struct pixel **my_search_image,
								int **hitbox, int original_dim_x, int original_dim_y,
								int search_dim_x, int search_dim_y, int start_x, int start_y,
								pthread_mutex_t*** hitbox_mutex, int total_threads)
{
	/* Variables used to calculate the weight of hue, saturation, and value */
	int scale_h = 4;
	int scale_s = 1;
	int scale_v = 1;

	/* Set the unreachable limits to get min and max for comparisons */
	double best_weighted = 65;
	double best_average = 100000;
	double best_total = 100000;

	/* Store the position of the best hash so far */
	int best_x = -1;
	int best_y = -1;

	int x = start_x;
	int y = start_y;

	double weight = 1.0 / (scale_h + scale_s + scale_v);

	double weight_h = weight * scale_h;
	double weight_s = weight * scale_s;
	double weight_v = weight * scale_v;

	/* Dimensions that we are iterating through inside the hitbox */
	int original_hash_x = original_dim_x - HASH_SIZE;
	int original_hash_y = original_dim_y - HASH_SIZE;

	while(y < search_dim_y)
	{
		/* Generate the hash for the current pixel chunk */
		struct hsv_hash my_hash = hash8_hsv_pixels(my_search_image, x, y);
		double my_avg_hue = my_hash.avg_hue;
		struct hsv corner_hsv = my_hash.corner;
		double my_hash2 = my_hash.hash2;
		struct pixel corner = my_search_image[y][x];

		/* Only check the hash if it passes the sky filter -- i.e. skip white/offwhite colors */
		if(sky_filter(&corner_hsv)==0)
		{
			/* Walk through the hit box to see which hashes in the original image is closest to it */
			for (int y2 = 0; y2 < original_hash_y; ++y2)
			{
				for (int x2 = 0; x2 < original_hash_x; ++x2)
				{
					/* Compare the search chunk hash to the hash in the original hash board */
					int ham_h = hamming_distance(&my_hash.h, &original_hashed_image[y2][x2].h);
					int ham_s = hamming_distance(&my_hash.s, &original_hashed_image[y2][x2].s);
					int ham_v = hamming_distance(&my_hash.v, &original_hashed_image[y2][x2].v);
					double check_weight = (weight_h * ham_h) + (weight_s * ham_s) + (weight_v * ham_v);
					double check_average = fabs(my_avg_hue - original_hashed_image[y2][x2].avg_hue);
					double check_total = fabs(my_hash2 - original_hashed_image[y2][x2].hash2);
					double sat_val_diff = fabs(original_hashed_image[y2][x2].corner.s-corner_hsv.s)+fabs(original_hashed_image[y2][x2].corner.v-corner_hsv.v);

					/* Filter out some of the results that are not optimal */
					if (check_weight < best_weighted
							&& fabs(corner_hsv.h - original_hashed_image[y2][x2].corner.h) < 20.0
							&& sat_val_diff<.3
							&& check_average < 10.0
							&& check_total < 10.0
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
			/* An extra filter to make sure that we are incrementing the hitbox only if the gradient matches */
			if (best_weighted < 10)
			{
				/* Lock the hitbox_mutex index to update the board without any synchronization issues */
				pthread_mutex_lock(&((*hitbox_mutex)[best_y][best_x]));
				hitbox[best_y][best_x] +=1;
				pthread_mutex_unlock(&((*hitbox_mutex)[best_y][best_x]));
			}
		}
		/* Redo the hash in another chunk (skipping by total_threads) -- bypasses the use of modulus */
		x+=(8*(total_threads+1));
		while(x>search_dim_x-HASH_SIZE-1)
		{
			x-=search_dim_x-HASH_SIZE-1;
			y+=8;
		}
		/* Reset the min/max values to be reused */
		best_weighted = 65;
		best_average = 100000;
		best_total = 100000;
		best_x = -1;
		best_y = -1;
	}
}


void* call_thread(void* args)
{
	/* Get the arguments the thread is responsible for */
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
	pthread_mutex_t ***hitbox_mutex = thread_params->hitbox_mutex;
	free(thread_params);

	/* Call the working function -- NOTE: varying start_x, start_y's let's the thread know which boxes to compute */
	hash_worker(original_hashed_image, my_search_image, hitbox, original_dim_x,
									original_dim_y, search_dim_x, search_dim_y, start_x, start_y,
									hitbox_mutex, total_threads);

	pthread_exit(NULL);

}

/* A filter that returns 0 on success (not the sky), -1 on failure (it is the sky -- white/offwhite colors) */
int sky_filter(struct hsv* corner)
{
	//near white
	if(corner->s<=.062 && corner->v>0.92)
	{
		return -1;
	}//blue?
	else if(corner->h<230 && corner->h>180)
	{
		if(corner->s<.1 && corner->v>0.82)
		{
			return -1;
		}
	}//filter grey clouds
	else if(corner->s<=.03 && corner->v>=.75)
	{
		return -1;
	}
	return 0;
}
