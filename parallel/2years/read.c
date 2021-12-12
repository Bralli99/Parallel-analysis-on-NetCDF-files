 #include <stdio.h>
 #include <string.h>
 #include <netcdf.h>
 #include <mpi.h>
 #include <omp.h>
 #include <sys/time.h>

 /* This is the name of the data file we will create. */
 #define FILE_NAME "/home/brando.chiminelli/exercises/Project/parallel/2years/average_2years.nc"
 #define NDIMS 3
 #define NLAT 160
 #define NLON 320
 #define LAT_NAME "lat"
 #define LON_NAME "lon"
 #define REC_NAME "time"
 #define TEMP_NAME "tasmin"
 #define UNITS "units"
 #define DEGREES_EAST "degrees_east"
 #define DEGREES_NORTH "degrees_north"
 #define UNITS "units"
 #define TEMP_UNITS "K"
 #define LAT_UNITS "degrees_north"
 #define LON_UNITS "degrees_east"
 #define MAX_ATT_LEN 80
  
 /* Handle errors by printing an error message and exiting with a non-zero status. */
 #define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

 int main(int argc, char *argv[])
 { 
    int nyears = 2;
    int NREC = 365 * nyears;

    char str[100];
    strcpy(str, "/shares/HPC4DataScience/indices/tasmin_day_EC-Earth3-Veg-LR_ssp585_r1i1p1f1_gr_");
    strcat(str, argv[1]);
    strcat(str, "0101-");
    strcat(str, argv[1]);
    strcat(str, "1231.nc");

    char str2[100];
    strcpy(str2, "/shares/HPC4DataScience/indices/tasmin_day_EC-Earth3-Veg-LR_ssp585_r1i1p1f1_gr_");
    strcat(str2, argv[2]);
    strcat(str2, "0101-");
    strcat(str2, argv[2]);
    strcat(str2, "1231.nc");

    /* IDs for the netCDF file, dimensions, and variables. */
    int ncid, lon_dimid, lat_dimid, rec_dimid;
    int lat_varid, lon_varid, temp_varid;
    int record = 1;
    int dimids[NDIMS];

    int ncid_r, temp_varid_r;
    int lat_varid_r, lon_varid_r;
  
    /* The start and count arrays will tell the netCDF library where to read our data. */
    size_t start[NDIMS], count[NDIMS];
  
    float temp_in[NLAT][NLON];
    float temp_out[NLAT][NLON];
    float temp_sum[NLAT][NLON];
  
    /* These program variables hold the latitudes and longitudes. */
    float lats[NLAT], lons[NLON];
  
    /* Loop indexes. */
    int k, i, lg, ln, rec, r;
    
    /* Error handling. */
    int retval;

    /* MPI define rank and nprocs */
    int rank, nprocs;

    /* Get time now */
    struct timeval now;
    gettimeofday(&now, NULL);

    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    /* Assign a single filename to a pool of procs */
    const char *filenames[nprocs]; 
    int poolprocs = nprocs/nyears;

    for (r = 0; r < nprocs; r++)
    { 
       if (r<(poolprocs)) {
         filenames[r] = str;
       } else {
          filenames[r] = str2;
       }
   }
  
    /* Open the file. */
    if ((retval = nc_open(filenames[rank], NC_NOWRITE, &ncid_r)))
       ERR(retval);
    
    /* Get the varids of the latitude and longitude coordinate variables. */
    if ((retval = nc_inq_varid(ncid_r, LAT_NAME, &lat_varid_r)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid_r, LON_NAME, &lon_varid_r)))
       ERR(retval);
  
    /* Read the coordinate variable data. */
    if ((retval = nc_get_var_float(ncid_r, lat_varid_r, &lats[0])))
       ERR(retval);
    if ((retval = nc_get_var_float(ncid_r, lon_varid_r, &lons[0])))
       ERR(retval);
  
    /* Get the varid of tasmin (netCDF variable). */
    if ((retval = nc_inq_varid(ncid_r, TEMP_NAME, &temp_varid_r)))
       ERR(retval);
  
    /* Set count and start to work on a subset of NREC (days) on each procs */
    count[1] = NLAT;
    count[2] = NLON;
    start[1] = 0;
    start[2] = 0;

    count[0] = NREC / nprocs;
    start[0] = count[0] * (rank % poolprocs);
    if (rank % poolprocs <= 365 % poolprocs) {
        start[0] += (rank % poolprocs);
        count[0]++;
    }
    else {
        start[0] += 365 % poolprocs;
    }
    if (count[0] == 0)
        start[0] = 0;
    
    /* Read and check one record at a time. */
    count[0] = record;

    /* Start reading from the netCDF file for a pool of days*/
    for (rec = 0; rec < NREC / nprocs; rec++)
    {
      /* Read a single day*/
      if ((retval = nc_get_vara_float(ncid_r, temp_varid_r, start, count, &temp_in[0][0])))
      ERR(retval);
      start[0]++;

      /* With OMP, sum the tasmin value */
      #pragma omp parallel for num_threads(4) private(i, k)
      for(i = 0; i < 160; i++)
      {
        for(k = 0; k < 320 ; k++)
        {
            temp_out[i][k] = temp_out[i][k] + temp_in[i][k];
            
        }
      }
    } /* next day */
   
   if ((retval = nc_close(ncid_r)))
   ERR(retval);

   /* Collect the different sums from each process on process 0 */
   MPI_Reduce(&temp_out, &temp_sum, 51200 , MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

   if(rank==0){

   
    /* With OMP, calculate the average on all the years */
    #pragma omp parallel for num_threads(4) private(ln, lg)
    for(ln = 0; ln < 160; ln++)
      {
        for(lg = 0; lg < 320 ; lg++)
        {
           temp_sum[ln][lg] = temp_sum[ln][lg]/ (365*nyears) ;
        }
      }

    /* Get the time and print the performance */
    struct timeval then;
    gettimeofday(&then, NULL);
    printf("Time: %ld\n", (then.tv_sec*1000000 + then.tv_usec) - (now.tv_sec*1000000 + now.tv_usec));
    
   
   /* Create the file. */
    if ((retval = nc_create(FILE_NAME, NC_CLOBBER, &ncid)))
       ERR(retval);

    if ((retval = nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)))
       ERR(retval);
    if ((retval = nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)))
       ERR(retval);
    if ((retval = nc_def_dim(ncid, REC_NAME, NC_UNLIMITED, &rec_dimid)))
       ERR(retval);

    if ((retval = nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, &lat_dimid, &lat_varid)))
       ERR(retval);
    if ((retval = nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, &lon_dimid, &lon_varid)))
       ERR(retval);
  
    if ((retval = nc_put_att_text(ncid, lat_varid, UNITS, strlen(DEGREES_NORTH), DEGREES_NORTH)))
       ERR(retval);
    if ((retval = nc_put_att_text(ncid, lon_varid, UNITS, strlen(DEGREES_EAST), DEGREES_EAST)))
       ERR(retval);
  
    dimids[0] = rec_dimid;
    dimids[1] = lat_dimid;
    dimids[2] = lon_dimid;

    if ((retval = nc_def_var(ncid, TEMP_NAME, NC_FLOAT, NDIMS, dimids, &temp_varid)))
       ERR(retval);

    if ((retval = nc_put_att_text(ncid, temp_varid, UNITS, strlen(TEMP_UNITS), TEMP_UNITS)))
       ERR(retval);
  
    if ((retval = nc_enddef(ncid)))
       ERR(retval);

    if ((retval = nc_put_var_float(ncid, lat_varid, &lats[0])))
       ERR(retval);
    if ((retval = nc_put_var_float(ncid, lon_varid, &lons[0])))
       ERR(retval);
      
    count[0] = 1;
    count[1] = NLAT;
    count[2] = NLON;
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
  
   if ((retval = nc_put_vara_float(ncid, temp_varid, start, count, &temp_sum[0][0])))
      ERR(retval);

   /* Close the file. */
    if ((retval = nc_close(ncid)))
       ERR(retval);
   }
   
   MPI_Finalize();
   return 0;
 }
