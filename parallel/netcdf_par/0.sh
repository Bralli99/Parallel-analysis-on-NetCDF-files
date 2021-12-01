#!/bin/bash
#PBS -l select=1:ncpus=4:mem=2gb
# set max execution time
#PBS -l walltime=0:01:00
# imposta la coda di esecuzione
#PBS -q short_cpuQ
module load mpich-3.2 hdf5-1.8.18
mpirun.actual -n 4 /home/brando.chiminelli/exercises/Project/parallel/netcdf_par/read
