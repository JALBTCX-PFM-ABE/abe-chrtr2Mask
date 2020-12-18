
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (int32_t *argc, char **argv, OPTIONS *op, QWidget *parent):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/chrtr2MaskWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("chrtr2Mask is a tool for masking land areas in a CHRTR2 file using the SRTM2 "
                                  "one-second world land mask.  A zero (0.0) value will be placed in each grid cell "
                                  "of the CHRTR2 file (that doesn't contain real or digitized data) through which "
                                  "the NGA 1:50,000 scale coastline passes.  Set the value to be used for masking in the "
                                  "obvious field below.  You can also, optionally, mask interpolated land values.<br><br>"
                                  "Help is available by clicking on the Help button and then "
                                  "clicking on the item for which you want help.  Select a CHRTR2 file below.  "
                                  "Click <b>Next</b> to continue or <b>Cancel</b> to exit.<br><br>"
                                  "<font color=\"#ff0000\"><b>IMPORTANT NOTE:  This program will not run if the SRTM2 "
                                  "land mask or the NGA 1:50,000 coastline is not present on your system.  Until further "
                                  "notice, use of the NGA 1:50,000 coastline and/or the SRTM2 one-second world land mask will "
                                  "cause the CHRTR2 file modified by this program to be classified as UNCLASSIFIED RESTRICTED, "
                                  "DoD and DoD Contractors Only at the very least.</b></font>"));
  label->setWordWrap (true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);


  QHBoxLayout *chrtr2_file_box = new QHBoxLayout (0);
  chrtr2_file_box->setSpacing (8);

  vbox->addLayout (chrtr2_file_box);


  QLabel *chrtr2_file_label = new QLabel (tr ("CHRTR2 File"), this);
  chrtr2_file_box->addWidget (chrtr2_file_label, 1);

  chrtr2_file_edit = new QLineEdit (this);
  chrtr2_file_edit->setReadOnly (true);
  chrtr2_file_box->addWidget (chrtr2_file_edit, 10);

  QPushButton *chrtr2_file_browse = new QPushButton (tr ("Browse..."), this);
  chrtr2_file_box->addWidget (chrtr2_file_browse, 1);

  chrtr2_file_label->setWhatsThis (chrtr2_fileText);
  chrtr2_file_edit->setWhatsThis (chrtr2_fileText);
  chrtr2_file_browse->setWhatsThis (chrtr2_fileBrowseText);

  connect (chrtr2_file_browse, SIGNAL (clicked ()), this, SLOT (slotCHRTR2FileBrowse ()));

  QGroupBox *oBox = new QGroupBox (tr ("Options"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);

  QGroupBox *landBox = new QGroupBox (this);
  QHBoxLayout *landBoxLayout = new QHBoxLayout;
  landBox->setLayout (landBoxLayout);

  maskLand = new QCheckBox (tr ("Mask interpolated land"), landBox);
  maskLand->setToolTip (tr ("Mask interpolated land data with a value (based on the grid spacing)"));
  maskLand->setWhatsThis (maskLandText);
  maskLand->setChecked (options->mask_interp_land);
  landBoxLayout->addWidget (maskLand);
  oBoxLayout->addWidget (landBox);


  QGroupBox *maskBox = new QGroupBox (this);
  QHBoxLayout *maskBoxLayout = new QHBoxLayout;
  maskBox->setLayout (maskBoxLayout);

  QLabel *maskLabel = new QLabel (tr ("Mask value"), maskBox);
  maskBoxLayout->addWidget (maskLabel);

  mask = new QDoubleSpinBox (this);
  mask->setDecimals (1);
  mask->setRange (-12000.0, 12000.0);
  mask->setSingleStep (100.0);
  mask->setValue (options->mask);
  mask->setWrapping (true);
  mask->setToolTip (tr ("Change the land mask value (-12000.0 to 12000.0)"));
  mask->setWhatsThis (maskText);
  maskBoxLayout->addWidget (mask);
  oBoxLayout->addWidget (maskBox);


  vbox->addWidget (oBox);


  if (*argc == 2)
    {
      CHRTR2_HEADER chrtr2_header;
      int32_t chrtr2_handle = -1;


      QString chrtr2_file_name = QString (argv[1]);


      char file[512];
      strcpy (file, chrtr2_file_name.toLatin1 ());

      chrtr2_handle = chrtr2_open_file (file, &chrtr2_header, CHRTR2_UPDATE);

      if (chrtr2_handle >= 0)
        {
          //  Save the min and max values so we don't try to insert a mask value that is outside the bounds.

          options->min_z = chrtr2_header.min_z;
          options->max_z = chrtr2_header.max_z;


	  chrtr2_file_edit->setText (chrtr2_file_name);

	  chrtr2_close_file (chrtr2_handle);
        }
    }


  if (!chrtr2_file_edit->text ().isEmpty ())
    {
      registerField ("chrtr2_file_edit", chrtr2_file_edit);
    }
  else
    {
      registerField ("chrtr2_file_edit*", chrtr2_file_edit);
    }

  registerField ("mask", mask, "value");
  registerField ("maskLand", maskLand);
}



void startPage::slotCHRTR2FileBrowse ()
{
  CHRTR2_HEADER       chrtr2_header;
  QStringList         files, filters;
  QString             file;
  int32_t             chrtr2_handle = -1;


  QFileDialog *fd = new QFileDialog (this, tr ("chrtr2Mask Open CHRTR2 File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->input_dir);


  filters << tr ("CHRTR2 (*.ch2)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("CHRTR2 (*.ch2)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString chrtr2_file_name = files.at (0);


      if (!chrtr2_file_name.isEmpty())
        {
          char file[512];

          strcpy (file, chrtr2_file_name.toLatin1 ());

          chrtr2_handle = chrtr2_open_file (file, &chrtr2_header, CHRTR2_UPDATE);

          if (chrtr2_handle < 0)
            {
              QMessageBox::warning (this, tr ("Open CHRTR2 File"),
				    tr ("The file ") + QDir::toNativeSeparators (chrtr2_file_name) + 
				    tr (" is not a CHRTR2 file or there was an error reading the file.") +
				    tr ("  The error message returned was:\n\n") +
				    QString (chrtr2_strerror ()));

	      return;
            }


          //  Save the min and max values so we don't try to insert a mask value that is outside the bounds.

          options->min_z = chrtr2_header.min_z;
          options->max_z = chrtr2_header.max_z;


	  chrtr2_close_file (chrtr2_handle);
        }


      chrtr2_file_edit->setText (chrtr2_file_name);

      options->input_dir = fd->directory ().absolutePath ();
    }
}
