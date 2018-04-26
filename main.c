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


int main(int argc, char* argv[])
{
  int mpi_numtasks,     /* number of tasks in partition */
      mpi_taskid;


  /* Intialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_numtasks);



  struct board* search = load_ppm("nick_jerry.ppm");
  struct board* original = load_ppm("wow.ppm");
  //resize_percent(&original,.5);


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
  float lower_bound = 0.1;
  int number_scales = 10; //we will be doing number_scales + 1 total
  float distance_between = (upper_bound - lower_bound)/number_scales;

  /* Storing the work load [rank_responsible_for_load][scales_responsible_for] */
  float **work_load = calloc(mpi_numtasks, sizeof(float *));
  for(int c1 = 0; c1 < mpi_numtasks; ++c1){
    work_load[c1] = calloc(number_scales, sizeof(float));
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
      printf("Rank %d - Total %f\n", c1, current_total[c1]);
      // int c2 = 0;
      // while(work_load[c1][c2] != 0){
      //   printf("%f ", work_load[c1][c2]);
      //   c2++;
      // }
      // printf("\n");
    }
  }

  struct best_score_info best_current_score;
  best_current_score.score = -1;
  best_current_score.search_start_x = -1;
  best_current_score.search_start_y = -1;
  best_current_score.dimension_x = -1;
  best_current_score.dimension_y = -1;
  best_current_score.total_hits = -1;


  int counter = 0;
  while(work_load[mpi_taskid][counter] != 0){
    struct board* copied_search = copy_board(search);
    float scale = work_load[mpi_taskid][counter];
    printf("Rank: %d -- Started scale: %f\n", mpi_taskid, scale);
    struct best_score_info result = find_image(&original,&copied_search, scale);
    if(result.score > best_current_score.score){
      best_current_score.score = result.score;
      best_current_score.search_start_x = result.search_start_x;
      best_current_score.search_start_y = result.search_start_y;
      best_current_score.dimension_x = result.dimension_x;
      best_current_score.dimension_y = result.dimension_y;
      best_current_score.total_hits = result.total_hits;
    }
    printf("Rank: %d -- Finished scale: %f\n", mpi_taskid, scale);
    counter += 1;
  }

  printf("Rank: %d finished computations\n", mpi_taskid);

  MPI_Barrier(MPI_COMM_WORLD);
  printf("Rank: %d -- Score: %f -- Total hits: %d -- Pos: (%d, %d) -- Size: (%d, %d)\n", mpi_taskid, best_current_score.score, best_current_score.total_hits, best_current_score.search_start_x, best_current_score.search_start_y, best_current_score.dimension_x, best_current_score.dimension_y);

  double local_res[2];
  double global_res[2];
  local_res[0]=best_current_score.score;
  local_res[1]=mpi_taskid;

  //we need to define a struct with a double and int
  MPI_Allreduce(local_res,global_res,1,MPI_DOUBLE_INT,MPI_MAXLOC,MPI_COMM_WORLD);

  printf("%f and %f\n",global_res[0],global_res[1]);

  if(mpi_taskid == (int)global_res[1])
  {
    printf("Rank %d has the best score\n", mpi_taskid);
    // bounding_box(&original,&best_current_score);
    // save_ppm(original,"boxed.ppm");
  }

  free_board(&search);
  free_board(&original);

  MPI_Finalize();
  return EXIT_SUCCESS;

}
