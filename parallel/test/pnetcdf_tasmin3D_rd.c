/*
 *  Copyright (C) 2003, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */
/* $Id$ */

/*
   This is an example which reads some 4D pressure and
   temperatures. The data file read by this program is produced by the
   companion program pres_temp_4D_wr.c. It is intended to illustrate
   the use of the netCDF C API.

   This program is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c

   $Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> /* basename() */
#include <pnetcdf.h>
#include <mpi.h>
#include </home/brando.chiminelli/exercises/Project/parallel/test/testutils.h>

/* This is the name of the data file we will read. */
#define FILE_NAME "/shares/HPC4DataScience/indices/tasmin_day_EC-Earth3-Veg-LR_ssp585_r1i1p1f1_gr_20570101-20571231.nc"

/* We are reading 4D data, a 2 x 6 x 12 lvl-lat-lon grid, with 2
   timesteps of data. */
#define NDIMS 3
#define NLAT 160
#define NLON 320
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define NREC 365
#define REC_NAME "time"

/* Names of things. */
#define TEMP_NAME "tasmin"
#define UNITS "units"
#define DEGREES_EAST "degrees_east"
#define DEGREES_NORTH "degrees_north"

/* For the units attributes. */
#define UNITS "units"
#define TEMP_UNITS "kelvin"
#define LAT_UNITS "degrees_north"
#define LON_UNITS "degrees_east"
#define MAX_ATT_LEN 80

int main(int argc, char **argv)
{
    int rank, nprocs, ncid, temp_varid;
    int lat_varid, lon_varid;

    /* The start and count arrays will tell the netCDF library where to
      read our data. */
    MPI_Offset start[NDIMS], count[NDIMS];

    /* Program variables to hold the data we will read. We will only
      need enough space to hold one timestep of data; one record. */
    float **temp_in = NULL; /* [NREC/nprocs][NLAT][NLON] */

    /* These program variables hold the latitudes and longitudes. */
    float lats[NLAT], lons[NLON];

    /* Loop indexes. */
    int i = 0;

    /* Error handling. */
    int err, nerrs = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Open the file. */
    err = ncmpi_open(MPI_COMM_WORLD, FILE_NAME, NC_NOWRITE, MPI_INFO_NULL, &ncid);
    CHECK_ERR

    if (rank == 0) {
        char *cmd_str = (char *)malloc(strlen(argv[0]) + 256);
        int format;
        err = ncmpi_inq_format(ncid, &format); CHECK_ERR
        if (format == NC_FORMAT_NETCDF4)
            sprintf(cmd_str, "*** TESTING C   %s for reading NetCDF-4 file", basename(argv[0]));
        else
            sprintf(cmd_str, "*** TESTING C   %s for reading classic file", basename(argv[0]));
        printf("%-66s ------ ", cmd_str);
        free(cmd_str);
    }


    /* Get the varids of the latitude and longitude coordinate variables. */
    err = ncmpi_inq_varid(ncid, LAT_NAME, &lat_varid);
    CHECK_ERR
    err = ncmpi_inq_varid(ncid, LON_NAME, &lon_varid);
    CHECK_ERR

    /* Read the coordinate variable data. */
    memset(lats, 0, sizeof(float) * NLAT);
    memset(lons, 0, sizeof(float) * NLON);
    err = ncmpi_get_var_float_all(ncid, lat_varid, &lats[0]);
    CHECK_ERR
    err = ncmpi_get_var_float_all(ncid, lon_varid, &lons[0]);
    CHECK_ERR

    /* Get the varids of the pressure and temperature netCDF
    * variables. */
    err = ncmpi_inq_varid(ncid, TEMP_NAME, &temp_varid);
    CHECK_ERR

    /* Read the data. Since we know the contents of the file we know
    * that the data arrays in this program are the correct size to
    * hold one timestep. */
    count[1] = NLAT;
    count[2] = NLON;
    start[1] = 0;
    start[2] = 0;

    /* divide NLVL dimension among processes */
    count[0] = NREC / nprocs; //365/nprocs = 5 (con 73 processi)
    start[0] = count[0] * rank;
    if (rank < NREC % nprocs) {
        start[0] += rank;
        count[0]++;
    }
    else {
        start[0] += NREC % nprocs;
    }
    if (count[0] == 0)
        start[0] = 0;

    /* allocate read buffers */
    temp_in = (float **)malloc(count[0] * sizeof(float *));
    if (count[0] > 0) {
        temp_in[0] = (float *)malloc(count[0] * NLAT * NLON * sizeof(float));
        for (i = 1; i < count[0]; i++) {
            temp_in[i] = temp_in[i - 1] + NLAT * NLON;
        }
    }

    /* Read and check one record at a time. */
        err = ncmpi_get_vara_float_all(ncid, temp_varid, start, count, &temp_in[0][0]);
        CHECK_ERR        

    /* Close the file. */
    err = ncmpi_close(ncid);
    CHECK_ERR

    /* check if there is any malloc residue */
    MPI_Offset malloc_size, sum_size;
    err = ncmpi_inq_malloc_size(&malloc_size);
    if (err == NC_NOERR) {
        MPI_Reduce(&malloc_size, &sum_size, 1, MPI_OFFSET, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0 && sum_size > 0)
            printf("heap memory allocated by PnetCDF internally has %lld bytes yet to be freed\n",
                   sum_size);
    }

    MPI_Allreduce(MPI_IN_PLACE, &nerrs, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (rank == 0) {
        if (nerrs)
            printf(FAIL_STR, nerrs);
        else
            printf(PASS_STR);
    }

    MPI_Finalize();
    return (nerrs > 0);
}
