#include "image.h"
#include "hash.h"
#include "hsv.h"
#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <mpi.h>
#include <float.h>
//#define BGQ 1 // when running BG/Q, comment out when running on kratos
//#define FILE_OUTPUT 1 //uncomment if you want file output for each rank. Requires correct directories
#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime
#endif

int main(int argc, char* argv[])
{
  int mpi_numtasks,     /* number of tasks in partition */
      mpi_taskid;


  /* Intialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_numtasks);

#ifdef FILE_OUTPUT
  char file_name[100];
  //make folders: output_<1/2/4/8/16/32/64/128/256..>
  sprintf(file_name, "/gpfs/u/home/PCP7/PCP7sztb/scratch/output_%d/taskid_%d.txt",mpi_numtasks, mpi_taskid);

  FILE *ptr;
  ptr = fopen(file_name, "w");
#endif

#ifndef BGQ
  //char* search_image = "./images/cutout.ppm";
  char* search_image = argv[2];
  char* original_image = argv[3];
  //char* original_image = "./images/eight.ppm";
#else
  //char* search_image = "/gpfs/u/home/PCP7/PCP7sztb/scratch/nick_jerry.ppm";
  //char* original_image = "/gpfs/u/home/PCP7/PCP7sztb/scratch/wow.ppm";
#endif

  double time_in_secs = 0;
  double processor_frequency = 1600000000.0;
  unsigned long long start_cycles=0;
  unsigned long long end_cycles=0;


  struct board* search = load_ppm(search_image);
  autocrop_board(&search, 255,255,255);
  struct board* original = load_ppm(original_image);
  //resize_percent(&original,.50);

  if(mpi_taskid == 0){
#ifdef FILE_OUTPUT
     fprintf(ptr, "Searching for %s in %s\n", search_image, original_image);
#endif
     printf("Searching for %s in %s\n", search_image, original_image);
  }
  /**
    Work distribution Problem:
      Given:
        n total sizes, and k machines
      Goal:
        minimize makespan (balance load as best as possible)
    Variables:
      Upper-bound (UB): max scaling bound
      Lower-bound (LB): lowest scaling bound
      Sizes (s): number of scales inbetween the upper-bound and lower-bound
    Derivation:
      Distance between scales:
        (UB-LB)/s
      Possible scales:
        ((UB-LB)/s)*i + LB, where i = 0 -> s
    =======================================
    Above is an NP-Hard, we will use an approximation algorithm to
      calculate the most even load. Sorted load-balancing has an
      approximation factor of (3/2)OPT
    Greedy Sorted Load-Balancing Algorithm:
      - Sort the jobs in decrementing order
      - Assign jobs to the machine with the least current load
    =======================================
  **/

  int search_dimx = search->resolution_x;
  int search_dimy = search->resolution_y;
  int original_dimx = original->resolution_x;
  int original_dimy = original->resolution_y;

  float max_scale_x = (float)original_dimx / search_dimx;
  float max_scale_y = (float)original_dimy / search_dimy;

  /* Define key variables for the problem */
  float upper_bound = min(max_scale_x, max_scale_y);
  float lower_bound = 0.3;
  int number_scales = strtol(argv[1],NULL,10); //we will be doing number_scales + 1 total
  float distance_between = (upper_bound - lower_bound)/number_scales;

  /* Storing the work load [rank_responsible_for_load][scales_responsible_for] */
  float **work_load = calloc(mpi_numtasks, sizeof(float *));
  for(int c1 = 0; c1 < mpi_numtasks; ++c1){
    work_load[c1] = calloc(number_scales+1, sizeof(float));
  }
  /* Store the current total to not have to loop through work_load[rank][n] twice */
  float *current_total = calloc(mpi_numtasks,sizeof(float *));

  /* Assign the jobs to the work_load array starting from largest job */
  for(int c1 = number_scales; c1 >= 0; --c1){
    float minimum_workload = FLT_MAX;
    int index_min_workload = -1;
    // loop through to find the machine with the minimum workload
    for(int c2 = 0; c2 < mpi_numtasks; ++c2){
      if(current_total[c2] < minimum_workload){
        minimum_workload = current_total[c2];
        index_min_workload = c2;
      }
    }
    // calculate the current_scale = LB + (distance_between * current_scale)
    float current_scale = lower_bound + (distance_between * c1);
    // place the current scale in the minimum_workload_machine
    int c2 = 0;
    while(work_load[index_min_workload][c2] != 0) c2 += 1;
    work_load[index_min_workload][c2] = current_scale;
    // update the current_total array
    current_total[index_min_workload] += current_scale * current_scale;
  }

  /* Print to check our distributed workload */
  if(mpi_taskid == 0){
    for(int c1 = 0; c1 < mpi_numtasks; c1++){
#ifdef FILE_OUTPUT
      fprintf(ptr, "Rank %d - Total %f\n", c1, current_total[c1]);
#endif
      printf("Rank %d - Total %f\n", c1, current_total[c1]);
    }
  }

  struct best_score_info best_current_score;
  best_current_score.score = -1;
  best_current_score.search_start_x = -1;
  best_current_score.search_start_y = -1;
  best_current_score.dimension_x = -1;
  best_current_score.dimension_y = -1;
  best_current_score.total_hits = -1;

  //START TIMING
  int counter = 0;
  start_cycles = GetTimeBase();
  /* Loop through the work assigned to the rank */
  while(work_load[mpi_taskid][counter] != 0){
    struct board* copied_search = copy_board(search);
    float scale = work_load[mpi_taskid][counter];
#ifdef FILE_OUTPUT
    fprintf(ptr,"Rank: %d -- Started scale: %f\n", mpi_taskid, scale);
#endif
    printf("Rank: %d -- Started scale: %f\n", mpi_taskid, scale);
    /* Do the entire computation and get the score! */
    struct best_score_info result = find_image(&original,&copied_search, scale);
#ifdef FILE_OUTPUT
    fprintf(ptr,"Rank: %d -- Score: %f -- Total hits: %d -- Pos: (%d, %d) -- Size: (%d, %d) -- Extra info: %f\n", mpi_taskid, result.score, result.total_hits, result.search_start_x, result.search_start_y, result.dimension_x, result.dimension_y, result.extra_info);
#endif
    printf("Rank: %d -- Score: %f -- Total hits: %d -- Pos: (%d, %d) -- Size: (%d, %d) -- Extra info: %f\n", mpi_taskid, result.score, result.total_hits, result.search_start_x, result.search_start_y, result.dimension_x, result.dimension_y, result.extra_info);
    /* Compare the current score with previous scores and get the best one */
    if(result.score > best_current_score.score){
      best_current_score.score = result.score;
      best_current_score.search_start_x = result.search_start_x;
      best_current_score.search_start_y = result.search_start_y;
      best_current_score.dimension_x = result.dimension_x;
      best_current_score.dimension_y = result.dimension_y;
      best_current_score.total_hits = result.total_hits;
    }
#ifdef FILE_OUTPUT
    fprintf(ptr,"Rank: %d -- Finished scale: %f\n", mpi_taskid, scale);
#endif
    printf("Rank: %d -- Finished scale: %f\n", mpi_taskid, scale);
    counter += 1;
    free_board(&copied_search);
  }
  end_cycles=GetTimeBase();
  time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;
#ifdef FILE_OUTPUT
  fprintf(ptr, "Rank: %d finished computations in %f seconds\n", mpi_taskid,time_in_secs);
#endif
  printf("Rank: %d finished computations in %f seconds\n", mpi_taskid,time_in_secs);

  MPI_Barrier(MPI_COMM_WORLD);
#ifdef FILE_OUTPUT
  fprintf(ptr,"Rank: %d -- Score: %f -- Total hits: %d -- Pos: (%d, %d) -- Size: (%d, %d)\n", mpi_taskid, best_current_score.score, best_current_score.total_hits, best_current_score.search_start_x, best_current_score.search_start_y, best_current_score.dimension_x, best_current_score.dimension_y);
#endif
  printf("Rank: %d -- Score: %f -- Total hits: %d -- Pos: (%d, %d) -- Size: (%d, %d)\n", mpi_taskid, best_current_score.score, best_current_score.total_hits, best_current_score.search_start_x, best_current_score.search_start_y, best_current_score.dimension_x, best_current_score.dimension_y);
  start_cycles=GetTimeBase();
  struct{
    double score;
    int rank;
  }local_res,global_res;

  local_res.score=best_current_score.score;
  local_res.rank=mpi_taskid;

  /* Reduce all scores into a single rank */
  MPI_Allreduce(&local_res,&global_res,1,MPI_DOUBLE_INT,MPI_MAXLOC,MPI_COMM_WORLD);

  /* If the max score is reduced to "my rank", draw onto the original image to show the outline */
  if(mpi_taskid == global_res.rank)
  {
    end_cycles=GetTimeBase();
    time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;
#ifdef FILE_OUTPUT
    fprintf(ptr,"Rank %d has the best score\n", mpi_taskid);
    fprintf(ptr, "Reduction took %f seconds\n",time_in_secs);
#endif
    printf("Rank %d has the best score\n", mpi_taskid);
    printf("Reduction took %f seconds\n",time_in_secs);
    /* Make the bounding box! */
    bounding_box(&original,&best_current_score);
    char boxed[100];
    sprintf(boxed, "./boxed_%d_ranks.ppm",mpi_numtasks);
    save_ppm(original,boxed);
  }

  free_board(&search);
  free_board(&original);

  free(current_total);
  for(int c1 = 0; c1 < mpi_numtasks; ++c1){
    free(work_load[c1]);
  }
  free(work_load);

#ifdef FILE_OUTPUT
  fclose(ptr);
#endif

  MPI_Finalize();
  return EXIT_SUCCESS;

}
