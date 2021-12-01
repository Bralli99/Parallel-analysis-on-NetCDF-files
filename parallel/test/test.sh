#!/bin/bash
#PBS -l select=4:ncpus=1:mem=2gb
# set max execution time
#PBS -l walltime=0:02:00
# imposta la coda di esecuzione
#PBS -q short_cpuQ
module load mpich-3.2
mpirun.actual -n 4 /home/brando.chiminelli/exercises/Project/parallel/test/pnetcdf_tasmin3D_rd
