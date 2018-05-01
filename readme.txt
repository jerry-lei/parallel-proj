# Project authors: Jerry Lei, Birch Sztabnik, Jonathan Cheng

# Project can be found on kratos @PPCsztabb under parallel/project
-- Under ~/parallel-proj/

# How to run:
- $ make fast
- $ make run

#arguments
#./a.out <scales> <search image> <original image>
#a matched image will be written with the name boxed_<num_of_ranks>_ranks.ppm

# To modify the number of scales, change the input argument.
# There is no restriction to number of ranks per tasks.
# The number of threads to use can be modified in hash.c in the defined variable 'NUMBER_THREADS'

#Test images
#pencil2.ppm as search and things.ppm as original is detected well at 8 scales
#taxi_cab.ppm as search and nyc_streets.ppm as original is detected at 8 scales
#face.ppm as search and computer.ppm as original is detected well at 16 scales
#palm3.ppm as search and pda_small.ppm as original is detected well at 4 scales
