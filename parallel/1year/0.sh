#!/bin/bash
#PBS -l select=16:ncpus=4:mem=2gb
# set max execution time
#PBS -l walltime=0:02:00
# imposta la coda di esecuzione
#PBS -q short_cpuQ
module load mpich-3.2 hdf5-1.10.5--gcc-9.1.0 netcdf-4.7.0--gcc-9.1.0
mpirun.actual -n 16 /home/brando.chiminelli/exercises/Project/parallel/1year/read 2057