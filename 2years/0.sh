#!/bin/bash
#PBS -l select=10:ncpus=4:mem=2gb
# set max execution time
#PBS -l walltime=0:02:00
# imposta la coda di esecuzione
#PBS -q short_cpuQ
module load mpich-3.2 hdf5-1.10.5--gcc-9.1.0 netcdf-4.7.0--gcc-9.1.0
mpirun.actual -n 10 /home/alessiojuan.depaoli/Project/2years/read