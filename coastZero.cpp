
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

    *****************************************  IMPORTANT NOTE  **********************************/



#include "chrtr2Mask.hpp"


//  This reads the NGA 1:50,000 coastline data and generates a CHRTR2_DIGITIZED_CONTOUR zero value in each bin that
//  the coastline passes through.

void coastZero (OPTIONS *options, int32_t chrtr2_handle, CHRTR2_HEADER *chrtr2_header, QGroupBox *mbox, QProgressBar *mbar)
{
  uint8_t **grid;


  //  Make a new MBR that is 2 grid points larger than the file (to make sure we get the edges right).

  NV_F64_XYMBR new_mbr;
  new_mbr.min_x = chrtr2_header->mbr.wlon;
  new_mbr.min_y = chrtr2_header->mbr.slat;
  new_mbr.max_x = chrtr2_header->mbr.elon;
  new_mbr.max_y = chrtr2_header->mbr.nlat;

  double x_grid = chrtr2_header->lon_grid_size_degrees;
  double y_grid = chrtr2_header->lat_grid_size_degrees;
  
  /*since we moved chrtr2 to grid registration we no longer need the half node shift, we need to magic number to ensure it works due to rounding -SJ */
  double half_x = chrtr2_header->lon_grid_size_degrees * EPS;
  double half_y = chrtr2_header->lat_grid_size_degrees * EPS;


  //  Compute the minimum cell size so that we can walk the coastline segments and make sure we hit all of the bins.

  double az, ss_cell_size_x, ss_cell_size_y;
  double center_map_x = chrtr2_header->mbr.wlon + (chrtr2_header->mbr.elon - chrtr2_header->mbr.wlon) / 2.0;
  double center_map_y = chrtr2_header->mbr.slat + (chrtr2_header->mbr.nlat - chrtr2_header->mbr.slat) / 2.0;

  invgp (NV_A0, NV_B0, center_map_y, center_map_x, center_map_y, center_map_x + chrtr2_header->lon_grid_size_degrees, &ss_cell_size_x, &az);
  invgp (NV_A0, NV_B0, center_map_y, center_map_x, center_map_y + chrtr2_header->lat_grid_size_degrees, center_map_x, &ss_cell_size_y, &az);

  double min_cell_size = qMin (ss_cell_size_x, ss_cell_size_y) / 2.0;


  //  Add a couple of cells to the new MBR.

  new_mbr.min_x -= x_grid * 2.0;
  new_mbr.min_y -= y_grid * 2.0;
  new_mbr.max_x += x_grid * 2.0;
  new_mbr.max_y += y_grid * 2.0;


  //  Compute the number of columns and rows based on the bin sizes.

  int32_t grid_cols = NINT ((new_mbr.max_x - new_mbr.min_x) / x_grid);
  int32_t grid_rows = NINT ((new_mbr.max_y - new_mbr.min_y) / y_grid);


  //  Allocate the grid.

  grid = (uint8_t **) malloc (grid_rows * sizeof (uint8_t *));
  if (grid == NULL)
    {
      QMessageBox::critical (0, chrtr2Mask::tr ("chrtr2Mask landfill"), chrtr2Mask::tr ("Error allocating grid array : %1").arg (strerror (errno)));
      exit (-1);
    }

  for (int32_t i = 0 ; i < grid_rows ; i++)
    {
      grid[i] = (uint8_t *) calloc (grid_cols, sizeof (uint8_t));
      if (grid[i] == NULL)
        {
          QMessageBox::critical (0, chrtr2Mask::tr ("chrtr2Mask landfill"), chrtr2Mask::tr ("Error allocating grid[i] array : %1").arg (strerror (errno)));
          exit (-1);
        }
    }


  //  Figure out which one-degree squares we need to retrieve from the coastline file.

  int32_t slat = (int32_t) (new_mbr.min_y + 90.0) - 90;
  int32_t nlat = (int32_t) (new_mbr.max_y + 90.0) - 89;
  int32_t wlon = (int32_t) (new_mbr.min_x + 180.0) - 180;
  int32_t elon = (int32_t) (new_mbr.max_x + 180.0) - 179;


  mbox->setTitle (chrtr2Mask::tr ("[Task 4 of %1] - Retrieving coastlines").arg (options->tasks));
  mbar->setRange (0, (nlat - slat) * (elon - wlon));
  qApp->processEvents ();


  double *coast_x, *coast_y, tmp_x[2], tmp_y[2];
  int32_t segCount;


  //  Loop through the one-degree squares.

  for (int32_t i = slat ; i < nlat ; i++)
    {
      for (int32_t j = wlon ; j < elon ; j++)
        {
          //  Read a piece of coastline.

          while ((segCount = read_coast (COAST_50K, j, i, &coast_x, &coast_y)))
            {
              //  Loop through each coastline segment in this piece.

              for (int32_t k = 1 ; k < segCount ; k++)
                {
                  if (options->dateline && coast_x[k] < 0.0) coast_x[k] += 360.0;


                  //  Clip to the MBR boundaries.  Don't clip the original points, clip a copy.

                  tmp_x[0] = coast_x[k - 1];
                  tmp_y[0] = coast_y[k - 1];
                  tmp_x[1] = coast_x[k];
                  tmp_y[1] = coast_y[k];


                  if (clip (&tmp_x[0], &tmp_y[0], &tmp_x[1], &tmp_y[1], new_mbr))
                    {
                      //  Compute the distance and azimuth from one end of the segment to the other.

                      double dist, az;
                      invgp (NV_A0, NV_B0, tmp_y[0], tmp_x[0], tmp_y[1], tmp_x[1], &dist, &az);


                      //  Figure out how many steps we'll have to use as we move along the segment so that we'll make sure that we
                      //  hit every intervening cell.

                      int32_t steps = (int32_t) (dist / min_cell_size);
                      if (!steps) steps = 1;


                      //  Move along the segment in min_cell_size increments.

                      for (int32_t m = 0 ; m < steps ; m++)
                        {
                          int32_t cell_x = (int32_t) ((tmp_x[0] - new_mbr.min_x) / x_grid);
                          int32_t cell_y = (int32_t) ((tmp_y[0] - new_mbr.min_y) / y_grid);


                          //  Set the cell to 1 (it's 0 by default).

                          if (cell_x >= 0 && cell_x < grid_cols && cell_y >= 0 && cell_y < grid_rows)
                            {
                              grid[cell_y][cell_x] = 1;


                              //  Add min_cell_size to the X and Y positions in the "az" direction (i.e. increment the position along the segment).

                              double lat, lon;
                              newgp (tmp_y[0], tmp_x[0], az, min_cell_size, &lat, &lon);


                              if (options->dateline && lon < 0.0) lon += 360.0;


                              tmp_y[0] = lat;
                              tmp_x[0] = lon;
                            }
                        }


                      //  Always get the last point.

                      int32_t cell_x = (int32_t) ((tmp_x[1] - new_mbr.min_x) / x_grid);
                      int32_t cell_y = (int32_t) ((tmp_y[1] - new_mbr.min_y) / y_grid);

                      if (cell_x >= 0 && cell_x < grid_cols && cell_y >= 0 && cell_y < grid_rows) grid[cell_y][cell_x] = 1;
                    }
                }


              //  Free the coastline memory.

              free (coast_x);
              free (coast_y);


              mbar->setValue (i * (elon - wlon) + j);
              qApp->processEvents ();
            }
        }
    }


  mbar->setValue ((nlat - slat) * (elon - wlon));
  qApp->processEvents ();


  //  Debug output.
  /*
    FILE *fp;
    fp = fopen ("fred.yxz", "w");

    for (int32_t i = 0 ; i < grid_rows ; i++)
    {
    fprintf (fp, "%03d ", i);
    double lat = new_mbr.min_y + (double) i * y_grid;

    for (int32_t j = 0 ; j < grid_cols ; j++)
    {
    double lon = new_mbr.min_x + (double) j * x_grid;

    fprintf (fp, "%01d", grid[i][j]);
    }
    fprintf (fp, "\n");
    }

    fclose (fp);
  */


  double min_y = chrtr2_header->mbr.slat;
  double max_y = chrtr2_header->mbr.nlat;
  double min_x = chrtr2_header->mbr.wlon;
  double max_x = chrtr2_header->mbr.elon;


  int32_t width = (NINT ((max_x - min_x) / chrtr2_header->lon_grid_size_degrees));
  int32_t height = (NINT ((max_y - min_y) / chrtr2_header->lat_grid_size_degrees));
  int32_t row = NINT ((min_y - chrtr2_header->mbr.slat) / chrtr2_header->lat_grid_size_degrees);
  int32_t column = NINT ((min_x - chrtr2_header->mbr.wlon) / chrtr2_header->lon_grid_size_degrees);

  if (column + width >= chrtr2_header->width)
    {
      width = chrtr2_header->width - column - 1;
      max_x = chrtr2_header->mbr.elon;
    }

  if (row + height >= chrtr2_header->height)
    {
      height = chrtr2_header->height - row - 1;
      max_y = chrtr2_header->mbr.nlat;
    }


  mbox->setTitle (chrtr2Mask::tr ("[Task 5 of %1] - Setting coastlines to zero").arg (options->tasks));
  mbar->setRange (0, grid_rows * grid_cols);
  qApp->processEvents ();


  CHRTR2_RECORD chrtr2_record;


  //  Now we have the coastline map so we need to map it back to the CHRTR2 grid and fill the points with the zero value.

  for (int32_t i = 0 ; i < grid_rows ; i++)
    {
      double lat = new_mbr.min_y + (double) i * y_grid + half_y;

      for (int32_t j = 0 ; j < grid_cols ; j++)
        {
          double lon = new_mbr.min_x + (double) j * x_grid + half_x;

          if (grid[i][j])
            {
              if (!chrtr2_read_record_lat_lon (chrtr2_handle, lat, lon, &chrtr2_record))
                {
                  //  Never fill real or digitized values.

                  if (!(chrtr2_record.status & (CHRTR2_REAL | CHRTR2_DIGITIZED_CONTOUR)))
                    {
                      chrtr2_record.z = 0.0;
                      chrtr2_record.status = (chrtr2_record.status & CHRTR2_DIGITIZED_MASK) | CHRTR2_DIGITIZED_CONTOUR;

                      chrtr2_write_record_lat_lon (chrtr2_handle, lat, lon, chrtr2_record);
                    }
                }


              mbar->setValue (i * grid_cols + j);
              qApp->processEvents ();
            }
        }
    }


  mbar->setValue (grid_rows * grid_cols);
  qApp->processEvents ();


  //  Free the coastline map.

  for (int32_t i = 0 ; i < grid_rows ; i++) free (grid[i]);
  free (grid);
}
