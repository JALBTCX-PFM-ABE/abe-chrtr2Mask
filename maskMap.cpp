
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


void maskMap (OPTIONS *options, int32_t chrtr2_handle, CHRTR2_HEADER *chrtr2_header, QGroupBox *mbox, QProgressBar *mbar)
{
  uint8_t **grid;
  float mask_value = (float) options->mask;


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
      QMessageBox::critical (0, chrtr2Mask::tr ("chrtr2Mask maskMap"), chrtr2Mask::tr ("Error allocating grid array : %1").arg (strerror (errno)));
      exit (-1);
    }

  for (int32_t i = 0 ; i < grid_rows ; i++)
    {
      grid[i] = (uint8_t *) calloc (grid_cols, sizeof (uint8_t));
      if (grid[i] == NULL)
        {
          QMessageBox::critical (0, chrtr2Mask::tr ("chrtr2Mask maskMap"), chrtr2Mask::tr ("Error allocating grid[i] array : %1").arg (strerror (errno)));
          exit (-1);
        }
    }


  //  Figure out which one-degree squares we need to retrieve from the srtm_mask or srtm_topo file.

  int32_t start_lat = (int32_t) (new_mbr.min_y + 90.0) - 90;
  int32_t end_lat = (int32_t) (new_mbr.max_y + 90.0) - 89;
  int32_t start_lon = (int32_t) (new_mbr.min_x + 180.0) - 180;
  int32_t end_lon = (int32_t) (new_mbr.max_x + 180.0) - 179;


  mbox->setTitle (chrtr2Mask::tr ("[Task 2 of %1] - Retrieving mask").arg (options->tasks));
  mbar->setRange (0, (end_lat - start_lat) * (end_lon - start_lon));
  qApp->processEvents ();


  double lat, lon;
  uint8_t *array = NULL;


  int32_t wsize = -1, hsize = 0;


  for (int32_t j = start_lat ; j <= end_lat ; j++)
    {
      for (int32_t k = start_lon ; k <= end_lon ; k++)
        {
          wsize = read_srtm_mask_one_degree (j, k, &array, 1);

          hsize = wsize;
          if (wsize == 1800) hsize = 3600;

          if (wsize > 2)
            {
              double winc = 1.0L / (double) wsize;
              double hinc = 1.0L / (double) hsize;

              for (int32_t m = 0 ; m < hsize ; m++)
                {
                  lat = (double) (j + 1) - ((double) m + 1.5L)  * hinc;

                  if (lat >= new_mbr.min_y && lat <= new_mbr.max_y)
                    {
                      int32_t lat_index = NINT ((lat - new_mbr.min_y) / y_grid);

                      if (lat_index >= 0 && lat_index < grid_rows)
                        {
                          for (int32_t n = 0 ; n < wsize ; n++)
                            {
                              if (array[m * wsize + n])
                                {
                                  lon = (double) k + ((double) n + 1.5L) * winc ;

                                  if (lon >= new_mbr.min_x && lon <= new_mbr.max_x)
                                    {
                                      int32_t lon_index = NINT ((lon - new_mbr.min_x) / x_grid);

                                      if (lon_index >= 0 && lon_index < grid_cols) grid[lat_index][lon_index] = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
          else
            {
              if (wsize == 1)
                {
                  wsize = hsize = 3600;

                  double winc = 1.0L / (double) wsize;
                  double hinc = 1.0L / (double) hsize;

                  for (int32_t m = 0 ; m < hsize ; m++)
                    {
                      lat = (double) (j + 1) - ((double) m + 1.5L)  * hinc;

                      if (lat >= new_mbr.min_y && lat <= new_mbr.max_y)
                        {
                          int32_t lat_index = NINT ((lat - new_mbr.min_y) / y_grid);

                          if (lat_index >= 0 && lat_index < grid_rows)
                            {
                              for (int32_t n = 0 ; n < wsize ; n++)
                                {
                                  lon = (double) k + ((double) n + 1.5L) * winc ;

                                  if (lon >= new_mbr.min_x && lon <= new_mbr.max_x)
                                    {
                                      int32_t lon_index = NINT ((lon - new_mbr.min_x) / x_grid);

                                      if (lon_index >= 0 && lon_index < grid_cols) grid[lat_index][lon_index] = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

          mbar->setValue ((j - start_lat) * (end_lon - start_lon) + (k - start_lon));
          qApp->processEvents ();
        }
    }

  mbar->setValue ((end_lat - start_lat) * (end_lon - start_lon));
  qApp->processEvents ();


  cleanup_srtm_mask ();


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


  mbox->setTitle (chrtr2Mask::tr ("[Task 3 of %1] - Setting mask value").arg (options->tasks));
  mbar->setRange (0, grid_rows * grid_cols);
  qApp->processEvents ();


  CHRTR2_RECORD chrtr2_record;


  //  Now we have the mask map so we need to map it back to the CHRTR2 grid and fill the points with the mask value.

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
                      chrtr2_record.z = mask_value;
                      chrtr2_record.status = (chrtr2_record.status & CHRTR2_DIGITIZED_MASK) | CHRTR2_LAND_MASK;
                      chrtr2_write_record_lat_lon (chrtr2_handle, lat, lon, chrtr2_record);
                    }
                }
            }

          mbar->setValue (i * grid_cols + j);
          qApp->processEvents ();
        }
    }


  mbar->setValue (grid_rows * grid_cols);
  qApp->processEvents ();


  //  Free the coastline map.

  for (int32_t i = 0 ; i < grid_rows ; i++) free (grid[i]);
  free (grid);
}
