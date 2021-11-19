/* Copyright 2006-2011 University Corporation for Atmospheric
 Research/Unidata. See COPYRIGHT file for conditions of use. */
 #include <stdio.h>
 #include <string.h>
 #include <netcdf.h>
  
 /* This is the name of the data file we will read. */
 #define FILE_NAME_R "/shares/HPC4DataScience/indices/tasmin_day_EC-Earth3-Veg-LR_ssp585_r1i1p1f1_gr_20570101-20571231.nc"

 /* This is the name of the data file we will create. */
 #define FILE_NAME "/home/alessiojuan.depaoli/Project/average.nc"
  
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
  

  /* Names of things. */
 #define UNITS "units"
 #define DEGREES_EAST "degrees_east"
 #define DEGREES_NORTH "degrees_north"
  
  
 /* For the units attributes. */
 #define UNITS "units"
 #define TEMP_UNITS "K"
 #define LAT_UNITS "degrees_north"
 #define LON_UNITS "degrees_east"
 #define MAX_ATT_LEN 80
  
 /* Handle errors by printing an error message and exiting with a
  * non-zero status. */
 #define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}
  
 int
 main()
 {
     /* IDs for the netCDF file, dimensions, and variables. */
    int ncid, lon_dimid, lat_dimid, lvl_dimid, rec_dimid;
    int lat_varid, lon_varid, pres_varid, temp_varid;
    int dimids[NDIMS];

    int ncid_r, temp_varid_r;
    int lat_varid_r, lon_varid_r;
  
    /* The start and count arrays will tell the netCDF library where to
       read our data. */
    size_t start[NDIMS], count[NDIMS];
  
    /* Program variables to hold the data we will read. We will only
       need enough space to hold one timestep of data; one record. */
    float temp_in[NLAT][NLON];
    float temp_out[NLAT][NLON];
  
    /* These program variables hold the latitudes and longitudes. */
    float lats[NLAT], lons[NLON];
  
    /* Loop indexes. */
    int k, i, lg, ln, rec;
    
    /* Error handling. */
    int retval;
  
    /* Open the file. */
    if ((retval = nc_open(FILE_NAME_R, NC_NOWRITE, &ncid_r)))
       ERR(retval);
  
    /* Get the varids of the latitude and longitude coordinate
     * variables. */
    if ((retval = nc_inq_varid(ncid_r, LAT_NAME, &lat_varid_r)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid_r, LON_NAME, &lon_varid_r)))
       ERR(retval);
  
    /* Read the coordinate variable data. */
    if ((retval = nc_get_var_float(ncid_r, lat_varid_r, &lats[0])))
       ERR(retval);
    if ((retval = nc_get_var_float(ncid_r, lon_varid_r, &lons[0])))
       ERR(retval);
  
    /* Get the varids of the pressure and temperature netCDF
     * variables. */
    if ((retval = nc_inq_varid(ncid_r, TEMP_NAME, &temp_varid_r)))
       ERR(retval);
  
    /* Read the data. Since we know the contents of the file we know
     * that the data arrays in this program are the correct size to
     * hold one timestep. */
    count[0] = 1;
    count[1] = NLAT;
    count[2] = NLON;
    
    start[1] = 0;
    start[2] = 0;
  
    /* Read and check one record at a time. */
    for (rec = 0; rec < NREC; rec++)
    {
       start[0] = rec;
       if ((retval = nc_get_vara_float(ncid_r, temp_varid_r, start,
                       count, &temp_in[0][0])))
      ERR(retval);  
      //printf("Temp in 0,0 = %f\n", temp_in[0][0]);

      for(i = 0; i < 160; i++)
      {
        for(k = 0; k < 320 ; k++)
        {
            temp_out[i][k] = temp_out[i][k] + temp_in[i][k];
        }
      }
    } /* next record */
    
    for(ln = 0; ln < 160; ln++)
      {
        for(lg = 0; lg < 320 ; lg++)
        {
           temp_out[ln][lg] = temp_out[ln][lg]/365;
        
        }
      }

    /* Close the file. */
    if ((retval = nc_close(ncid_r)))
       ERR(retval);
  
    printf("*** SUCCESS reading example file pres_temp_4D.nc!\n");


   /* Create the file. */
    if ((retval = nc_create(FILE_NAME, NC_CLOBBER, &ncid)))
       ERR(retval);
  
    /* Define the dimensions. The record dimension is defined to have
     * unlimited length - it can grow as needed. In this example it is
     * the time dimension.*/

   
    if ((retval = nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)))
       ERR(retval);
    if ((retval = nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)))
       ERR(retval);
    if ((retval = nc_def_dim(ncid, REC_NAME, NC_UNLIMITED, &rec_dimid)))
       ERR(retval);

   /* Define the coordinate variables. We will only define coordinate
       variables for lat and lon.  Ordinarily we would need to provide
       an array of dimension IDs for each variable's dimensions, but
       since coordinate variables only have one dimension, we can
       simply provide the address of that dimension ID (&lat_dimid) and
       similarly for (&lon_dimid). */

    if ((retval = nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, &lat_dimid,
                 &lat_varid)))
       ERR(retval);
    if ((retval = nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, &lon_dimid,
                 &lon_varid)))
       ERR(retval);
  
    /* Assign units attributes to coordinate variables. */
    if ((retval = nc_put_att_text(ncid, lat_varid, UNITS,
                  strlen(DEGREES_NORTH), DEGREES_NORTH)))
       ERR(retval);
    if ((retval = nc_put_att_text(ncid, lon_varid, UNITS,
                  strlen(DEGREES_EAST), DEGREES_EAST)))
       ERR(retval);
  
  /* The dimids array is used to pass the dimids of the dimensions of
       the netCDF variables. Both of the netCDF variables we are
       creating share the same four dimensions. In C, the
       unlimited dimension must come first on the list of dimids. */
    dimids[0] = rec_dimid;
    dimids[1] = lat_dimid;
    dimids[2] = lon_dimid;

    /* Define the netCDF variables for the pressure and temperature
     * data. */
   
    if ((retval = nc_def_var(ncid, TEMP_NAME, NC_FLOAT, NDIMS,
                 dimids, &temp_varid)))
       ERR(retval);
  
    /* Assign units attributes to the netCDF variables. */

    if ((retval = nc_put_att_text(ncid, temp_varid, UNITS,
                  strlen(TEMP_UNITS), TEMP_UNITS)))
       ERR(retval);
  
    /* End define mode. */
    if ((retval = nc_enddef(ncid)))
       ERR(retval);

   
   /* Write the coordinate variable data. This will put the latitudes
       and longitudes of our data grid into the netCDF file. */
    if ((retval = nc_put_var_float(ncid, lat_varid, &lats[0])))
       ERR(retval);
    if ((retval = nc_put_var_float(ncid, lon_varid, &lons[0])))
       ERR(retval);
  
    /* These settings tell netcdf to write one timestep of data. (The
      setting of start[0] inside the loop below tells netCDF which
      timestep to write.) */
    count[0] = 1;
    count[1] = NLAT;
    count[2] = NLON;
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
  
   if ((retval = nc_put_vara_float(ncid, temp_varid, start, count,
                       &temp_out[0][0])))
      ERR(retval);


   /* Close the file. */
    if ((retval = nc_close(ncid)))
       ERR(retval);
  
    printf("*** SUCCESS writing example file %s!\n", FILE_NAME);

    return 0;
 }