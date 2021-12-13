# HPC Project

To test the project:

- load this modules -> module load mpich-3.2 hdf5-1.10.5--gcc-9.1.0 netcdf-4.7.0--gcc-9.1.0
- change two paths -> the first one in the 0.sh file for the executable and the second in the read.c file for the netcdf output position.
- to build the serial one: gcc -g -o read read.c -I/apps/netCDF4.7.0/include -L/apps/netCDF4.7.0/lib -lnetcdf -lm -ldl -lz -lcurl
- to build the parallel one: mpicc -g -Wall -fopenmp -o read read.c -I/apps/netCDF4.7.0--gcc-9.1.0/include -I/apps/hdf5-1.10.5--gcc-9.1.0/include -L/apps/netCDF4.7.0--gcc-9.1.0/lib -L/apps/hdf5-1.10.5--gcc-9.1.0/lib -lnetcdf -lm -ldl -lz -lcurl -lhdf5 -lhdf5_hl
- to submit the job to the PBS scheduler -> qsub 0.sh 
