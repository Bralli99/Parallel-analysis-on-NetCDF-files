/* Copyright 2006-2011 University Corporation for Atmospheric
 Research/Unidata. See COPYRIGHT file for conditions of use. */
 #include <stdio.h>
 #include <string.h>
 #include <netcdf.h>
  
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
  
 /* These are used to calculate the values we expect to find. */
 #define SAMPLE_TEMP 9.0
 #define START_LAT -89.1415194264611
 #define START_LON 0
  
 /* Handle errors by printing an error message and exiting with a
  * non-zero status. */
 #define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}
  
 int
 main()
 {
    int ncid, temp_varid;
    int lat_varid, lon_varid;
  
    /* The start and count arrays will tell the netCDF library where to
       read our data. */
    size_t start[NDIMS], count[NDIMS];
  
    /* Program variables to hold the data we will read. We will only
       need enough space to hold one timestep of data; one record. */
    float temp_in[NLAT][NLON];
  
    /* These program variables hold the latitudes and longitudes. */
    float lats[NLAT], lons[NLON];
  
    /* Loop indexes. */
    int k, i, rec, conta = 0, t;
    float somma[51200];
    
    /* Error handling. */
    int retval;
  
    /* Open the file. */
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
       ERR(retval);
  
    /* Get the varids of the latitude and longitude coordinate
     * variables. */
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
       ERR(retval);
  
    /* Read the coordinate variable data. */
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
       ERR(retval);
    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
       ERR(retval);
  
    /* Get the varids of the pressure and temperature netCDF
     * variables. */
    if ((retval = nc_inq_varid(ncid, TEMP_NAME, &temp_varid)))
       ERR(retval);
  
    /* Read the data. Since we know the contents of the file we know
     * that the data arrays in this program are the correct size to
     * hold one timestep. */
    count[0] = 1;
    count[1] = 1;
    count[2] = NLAT;
    count[3] = NLON;
    start[1] = 0;
    start[2] = 0;
    start[3] = 0;
  
    /* Read and check one record at a time. */
    for (rec = 0; rec < NREC; rec++)
    {
       start[0] = rec;
       if ((retval = nc_get_vara_float(ncid, temp_varid, start,
                       count, &temp_in[0][0])))
      ERR(retval);  
      //printf("Temp in 0,0 = %f\n", temp_in[0][0]);
      for(i = 0; i < 160; i++)
      {
        for(k = 0; k < 320 ; k++)
        {
            somma[conta] = somma[conta] + temp_in[i][k];
            conta++;
        }
      }
    } /* next record */
    for(t = 0; t < 51200 ; t++)
        {
           printf("%f\n", somma[t]/365);
        }

    /* Close the file. */
    if ((retval = nc_close(ncid)))
       ERR(retval);
  
    printf("*** SUCCESS reading example file pres_temp_4D.nc!\n");
    return 0;
 }