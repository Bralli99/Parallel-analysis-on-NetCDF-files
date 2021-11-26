#!/bin/bash
#PBS -l select=4:ncpus=1:mem=2gb
# set max execution time
#PBS -l walltime=0:01:00
# imposta la coda di esecuzione
#PBS -q short_cpuQ
module load mpich-3.2 hdf5-1.10.5--gcc-9.1.0 netcdf-4.7.0--gcc-9.1.0
mpirun.actual -n 4 /home/brando.chiminelli/exercises/Project/parallel/test/pres_temp_4D_rd
