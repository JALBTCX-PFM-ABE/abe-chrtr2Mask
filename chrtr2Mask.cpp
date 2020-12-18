
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
#include "chrtr2MaskHelp.hpp"


#define         FILTER 9


double settings_version = 1.20;


chrtr2Mask::chrtr2Mask (int32_t *argc, char **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/chrtr2MaskWatermark.png"));


  //  Get the user's defaults if available

  envin (&options);


  // Set the application font

  QApplication::setFont (options.font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, &options, this));

  setPage (1, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start masking the CHRTR2 file"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


chrtr2Mask::~chrtr2Mask ()
{
}



void chrtr2Mask::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);


  switch (id)
    {
    case 0:
      break;

    case 1:

      options.mask = field ("mask").toDouble ();
      options.mask_interp_land = field ("maskLand").toBool ();


      //  Check the mask value.

      if (options.mask < options.min_z || options.mask > options.max_z)
        {
          QMessageBox::critical (this, tr ("chrtr2Mask"),
                                 tr ("The mask value (%1) is outside of the CHRTR2 Z bounds (%2 to %3).  Please correct.").arg
                                 (options.mask, 0, 'f', 2).arg (options.min_z, 0, 'f', 2).arg (options.max_z, 0, 'f', 2));
        }
      else
        {
          button (QWizard::CustomButton1)->setEnabled (true);

          chrtr2_file_name = field ("chrtr2_file_edit").toString ();


          //  Use frame geometry to get the absolute x and y.

          QRect tmp = this->frameGeometry ();
          options.window_x = tmp.x ();
          options.window_y = tmp.y ();


          //  Use geometry to get the width and height.

          tmp = this->geometry ();
          options.window_width = tmp.width ();
          options.window_height = tmp.height ();


          //  Save the options.

          envout (&options);


          QString string;

          checkList->clear ();

          string = tr ("Input CHRTR2 file : ") + chrtr2_file_name;
          checkList->addItem (string);

          string.sprintf (tr ("SRTM mask value : %.2f").toLatin1 (), options.mask);
          checkList->addItem (string);
        }
      break;
    }
}



void chrtr2Mask::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;
    }
}



void chrtr2Mask::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
chrtr2Mask::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  int32_t             chrtr2_handle;
  CHRTR2_HEADER       chrtr2_header;
  CHRTR2_RECORD       chrtr2_record;
  QString             string;
  char                chrtr2_file[512];
  mispThread          misp_thread;
  
  
  void maskMap (OPTIONS *options, int32_t chrtr2_handle, CHRTR2_HEADER *chrtr2_header, QGroupBox *mbox, QProgressBar *mbar);
  void coastZero (OPTIONS *options, int32_t chrtr2_handle, CHRTR2_HEADER *chrtr2_header, QGroupBox *mbox, QProgressBar *mbar);
  
  
  QApplication::setOverrideCursor (Qt::WaitCursor);
  
  
  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);
  
  
  strcpy (chrtr2_file, chrtr2_file_name.toLatin1 ());
  
  
  chrtr2_handle = chrtr2_open_file (chrtr2_file, &chrtr2_header, CHRTR2_UPDATE);
  
  
  //  Check for dateline crossing.
  
  options.dateline = NVFalse;
  if (chrtr2_header.mbr.elon > 180.0 && chrtr2_header.mbr.wlon < 180.0) options.dateline = NVTrue;
  
  
  //  Check to see if the land mask is available.
  
  if (check_srtm_mask (1) != NULL)
    {
      QMessageBox::critical (this, tr ("chrtr2Mask"), tr ("The SRTM mask is not avalable\n\n"));
      
      exit (-1);
    }
  
  options.tasks = 8;
  if (options.mask_interp_land) options.tasks = 9;
  
  progress.mbox->setTitle (tr ("[Task 1 of %1] - Clearing old land mask data").arg (options.tasks));
  progress.mbar->setRange (0, chrtr2_header.height);
  qApp->processEvents ();
  
  
  float min_z = 999999999.0, max_z = -999999999.0;
  
  for (int32_t i = 0 ; i < chrtr2_header.height ; i++)
    {
      NV_I32_COORD2 coord;
      
      coord.y = i;
      
      
      for (int32_t j = 0 ; j < chrtr2_header.width ; j++)
        {
          coord.x = j;
          
          chrtr2_read_record (chrtr2_handle, coord, &chrtr2_record);
          
          
          //  Clear previously land masked data.  Note that we never land mask real or digitized data so we can safely set these to NULL.
          
          if (chrtr2_record.status & CHRTR2_LAND_MASK)
            {
              memset (&chrtr2_record, 0, sizeof (CHRTR2_RECORD));
              
              chrtr2_write_record (chrtr2_handle, coord, chrtr2_record);
            }
          else
            {
              if (chrtr2_record.status)
                {
                  min_z = qMin (chrtr2_record.z, min_z);
                  max_z = qMax (chrtr2_record.z, max_z);
                }
            }
        }
      
      progress.mbar->setValue (i);
      qApp->processEvents ();
    }
  
  progress.mbar->setValue (chrtr2_header.height);
  qApp->processEvents ();
  
  
  //  Mask using the SRTM2 one-second world land mask.
  
  maskMap (&options, chrtr2_handle, &chrtr2_header, progress.mbox, progress.mbar);
  
  
  //  Add zero value data at coastline.  The coastline type is based on the mask source.  NGA 1:50,000 for SRTM topo or mask
  //  (since the coastline was derived from the SRTM data) or WVS 1:250,000 for the WVS mask.
  
  /*  SJ - 02/14/2013 - landmasking should only produce 1 or mask_value*/
  //  coastZero (&options, chrtr2_handle, &chrtr2_header, progress.mbox, progress.mbar);
  
  
  //  Now we need to regrid.  Define the MBR for the new grid (adding the filter border).
  
  NV_F64_XYMBR mbr;
  mbr.min_x = chrtr2_header.mbr.wlon;
  mbr.min_y = chrtr2_header.mbr.slat;
  mbr.max_x = chrtr2_header.mbr.elon;
  mbr.max_y = chrtr2_header.mbr.nlat;
  
  
  /*  Add the filter border to the MBR.  */
  
  mbr.min_x -= ((double) FILTER * chrtr2_header.lon_grid_size_degrees);
  mbr.min_y -= ((double) FILTER * chrtr2_header.lat_grid_size_degrees);
  mbr.max_x += ((double) FILTER * chrtr2_header.lon_grid_size_degrees);
  mbr.max_y += ((double) FILTER * chrtr2_header.lat_grid_size_degrees);
  
  
  /*  Number of rows and columns in the area  */
  
  int32_t grid_rows = NINT ((mbr.max_y - mbr.min_y) / chrtr2_header.lat_grid_size_degrees);
  int32_t grid_cols = NINT ((mbr.max_x - mbr.min_x) / chrtr2_header.lon_grid_size_degrees);
  
  
  int32_t row_filter = grid_rows - FILTER;
  int32_t col_filter = grid_cols - FILTER;
  
  
  NV_I32_COORD2 coord;
  
  /*  We're going to let MISP/SURF handle everything in zero based units of the bin size.  That is, we subtract off the
      west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
      the south latitude.  This will give us values that range from 0.0 to grid5_cols in longitude and 0.0 to
      grid5_rows in latitude.  */
  
  
  
  /* For the time being we are not remisping, but keep the code in case we ever decide to put it back in -SJ

  misp_mbr.min_x = 0.0;
  misp_mbr.min_y = 0.0;
  misp_mbr.max_x = (double) grid_cols;
  misp_mbr.max_y = (double) grid_rows;


  misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, -2, misp_mbr);



  progress.mbox->setTitle (tr ("[Task 6 of %1] - Reading data for regrid").arg (options.tasks));
  progress.mbar->setRange (0, chrtr2_header.height);
  qApp->processEvents ();


  for (int32_t i = 0 ; i < chrtr2_header.height ; i++)
    {
      coord.y = i;

      for (int32_t j = 0 ; j < chrtr2_header.width ; j++)
        {
          coord.x = j;

          chrtr2_read_record (chrtr2_handle, coord, &chrtr2_record);

          if (chrtr2_record.status & (CHRTR2_REAL | CHRTR2_DIGITIZED_CONTOUR))
            {
              chrtr2_get_lat_lon (chrtr2_handle, &xy.y, &xy.x, coord);


              //  Load the points.

                  IMPORTANT NOTE:  MISP and GMT (by default) grid using corner posts.  That is, the data in a bin is assigned to the 
                  lower left corner of the bin.  Normal gridding/binning systems use the center of the bin.  Because of this we need
                  to lie to MISP/GMT and tell them that the point is really half a bin lower and to the left.  This is extremely
                  confusing but it works.
              //


              //  since chrtr2 moved to grid registraion we no longer need the half node shift -SJ//

              xyz.x = (xy.x - mbr.min_x) / chrtr2_header.lon_grid_size_degrees;// - 0.5;
              xyz.y = (xy.y - mbr.min_y) / chrtr2_header.lat_grid_size_degrees;// - 0.5;
              xyz.z = chrtr2_record.z;
     
              misp_load (xyz);
            }
        }
     
      progress.mbar->setValue (i);
      qApp->processEvents ();
    }


  progress.mbar->setValue (chrtr2_header.height);
  qApp->processEvents ();
     
     
  progress.mbox->setTitle (tr ("[Task 7 of %1] - Regridding data (please be patient)").arg (options.tasks));
  progress.mbar->setRange (0, 0);
  progress.mbar->setValue (-1);
  qApp->processEvents ();
     
     
  //  We're starting the MISP processing concurrently using a thread.  Note that we're using the Qt::DirectConnection type
  //  for the signal/slot connections.  This causes all of the signals emitted from the thread to be serviced immediately.
  //  Why are we running misp_proc in a thread???  Because it's the only way to get the stupid progress bar to update so
  //  that the user will know that the damn program is still running.  Sheesh!
     
  complete = NVFalse;
  connect (&misp_thread, SIGNAL (completed ()), this, SLOT (slotMispCompleted ()), Qt::DirectConnection);
     
  misp_thread.misp ();
     
     
  //  We can't move on until the thread is complete but we want to keep our progress bar updated.  This is a bit tricky 
  //  because you can't update the progress bar from within slots connected to thread signals.  Those slots are considered part
  //  of the mispThread and not part of the GUI thread.  When the thread is finished we move on to the retrieval step.
     
  while (!complete)
    {
#ifdef NVWIN3X
      Sleep (50);
#else
      usleep (50000);
#endif
     
      qApp->processEvents ();
    }
     
  */


  progress.mbox->setTitle (tr ("[Task 8 of %1] - Retrieving grid data").arg (options.tasks));
  progress.mbar->setRange (0, grid_rows);
  qApp->processEvents ();
  
  
  float *array = (float *) malloc ((grid_cols + 1) * sizeof (float));
  
  if (array == NULL)
    {
      perror ("Allocating array in main.c");
      exit (-1);
    }


  min_z = 9999999999.0;
  max_z = -9999999999.0;


  //  This is where we stuff the new interpolated surface in to the new CHRTR2.

  for (int32_t i = 0 ; i < grid_rows ; i++)
    {
      //if (!misp_rtrv (array)) break;



      //  Only use data that aren't in the filter border

      if (i >= FILTER && i < row_filter)
        {
          coord.y = i - FILTER;

          for (int32_t j = 0 ; j < grid_cols ; j++)
            {
              //  Only use data that aren't in the filter border

              if (j >= FILTER && j < col_filter)
                {
                  coord.x = j - FILTER;


                  //  Make sure we're inside the CHRTR2 bounds.

                  if (coord.y >= 0 && coord.y < chrtr2_header.height && coord.x >= 0 && coord.x < chrtr2_header.width)
                    {
                      chrtr2_read_record (chrtr2_handle, coord, &chrtr2_record);


                      //  Don't replace real, hand-drawn/digitized, or land masked data.  //
                      /*
                        if (!(chrtr2_record.status & (CHRTR2_REAL | CHRTR2_DIGITIZED_CONTOUR | CHRTR2_LAND_MASK)))
                        {
                        chrtr2_record.z = array[j];
                        chrtr2_record.status |= CHRTR2_INTERPOLATED;
                        }
                      */

                      min_z = MIN (chrtr2_record.z, min_z);
                      max_z = MAX (chrtr2_record.z, max_z);

                      if(chrtr2_record.status == CHRTR2_NULL || chrtr2_record.z>0)
                        {
                          chrtr2_record.z = 1;
                          chrtr2_record.status = (chrtr2_record.status & CHRTR2_INTERPOLATED_MASK) | CHRTR2_INTERPOLATED;
                        }
                      chrtr2_write_record (chrtr2_handle, coord, chrtr2_record);
                    }
                }
            }
        }

      progress.mbar->setValue (i);
      qApp->processEvents ();
    }


  progress.mbar->setValue (grid_rows);
  qApp->processEvents ();


  free (array);


  //  Now mask interpolated land data if we asked for it.

  if (options.mask_interp_land)
    {
      float force_value = 0.0;
      if (chrtr2_header.lat_grid_size_degrees > 0.08333166666667)
        {
          force_value = 20.0;
        }
      else if (chrtr2_header.lat_grid_size_degrees > 0.0333166666667 && chrtr2_header.lat_grid_size_degrees <= 0.08333166666667)
        {
          force_value = 10.0;
        }
      else if (chrtr2_header.lat_grid_size_degrees > 0.01665 && chrtr2_header.lat_grid_size_degrees <= 0.0333166666667)
        {
          force_value = 10.0;
        }
      else if (chrtr2_header.lat_grid_size_degrees > 0.00831666666667 && chrtr2_header.lat_grid_size_degrees <= 0.01665)
        {
          force_value = 4.0;
        }
      else
        {
          force_value = 2.0;
        }


      progress.mbox->setTitle (tr ("[Task 9 of %1] - Masking interpolated land data").arg (options.tasks));
      progress.mbar->setRange (0, chrtr2_header.height);
      qApp->processEvents ();

      for (int32_t i = 0 ; i < chrtr2_header.height ; i++)
        {
          coord.y = i;

          for (int32_t j = 0 ; j < chrtr2_header.width ; j++)
            {
              coord.x = j;

              if (!chrtr2_read_record (chrtr2_handle, coord, &chrtr2_record))
                {
                  //  Never fill real or digitized values.
                  if (!(chrtr2_record.status & (CHRTR2_REAL | CHRTR2_DIGITIZED_CONTOUR | CHRTR2_LAND_MASK)))
                    {
                      if (chrtr2_record.z < 0.0)
                        {
                          chrtr2_record.z = force_value;
                          chrtr2_record.status = (chrtr2_record.status & CHRTR2_DIGITIZED_MASK) | CHRTR2_LAND_MASK;
                          chrtr2_write_record (chrtr2_handle, coord, chrtr2_record);
                        }
                    }
                }
            }

          progress.mbar->setValue (i);
          qApp->processEvents ();
        }

      progress.mbar->setValue (chrtr2_header.height);
      qApp->processEvents ();
    }


  chrtr2_close_file (chrtr2_handle);


  //  Update the header with the observed min and max values.

  chrtr2_header.min_observed_z = min_z;
  chrtr2_header.max_observed_z = max_z;

  chrtr2_update_header (chrtr2_handle, chrtr2_header);

  chrtr2_close_file (chrtr2_handle);


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Masking complete, press Finish to exit."));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);
}



void chrtr2Mask::slotMispCompleted ()
{
  complete = NVTrue;
}



//  Get the users defaults.

void chrtr2Mask::envin (OPTIONS *options)
{
  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  options->font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = options->font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options->font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 1.00;


  // Set defaults so that if keys don't exist the parameters are defined

  options->mask = -5.0;
  options->mask_interp_land = NVTrue;
  options->input_dir = ".";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 1000;
  options->window_height = 500;


#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/chrtr2Mask.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/chrtr2Mask.ini";
#endif
  QSettings settings (ini_file, QSettings::IniFormat);

  settings.beginGroup (tr ("chrtr2Mask"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->mask = settings.value (tr ("mask"), options->mask).toDouble ();
  options->mask_interp_land = settings.value (tr ("mask interpolated land"), options->mask_interp_land).toBool ();
  options->input_dir = settings.value (tr ("input directory"), options->input_dir).toString ();

  options->window_width = settings.value (tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void chrtr2Mask::envout (OPTIONS *options)
{
#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/chrtr2Mask.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/chrtr2Mask.ini";
#endif
  QSettings settings (ini_file, QSettings::IniFormat);


  settings.beginGroup (tr ("chrtr2Mask"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("mask"), options->mask);
  settings.setValue (tr ("mask interpolated land"), options->mask_interp_land);
  settings.setValue (tr ("input directory"), options->input_dir);

  settings.setValue (tr ("width"), options->window_width);
  settings.setValue (tr ("height"), options->window_height);
  settings.setValue (tr ("x position"), options->window_x);
  settings.setValue (tr ("y position"), options->window_y);

  settings.endGroup ();
}
