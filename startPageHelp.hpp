
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



QString chrtr2_fileText = 
  startPage::tr ("Use the browse button to select an input CHRTR2 file.  You cannot modify the text in the "
		 "<b>CHRTR2 File</b> text window.  The reason for this is that the file must exist in order for the "
		 "program to run.  Note also that the <b>Next</b> button will not work until you select an input "
		 "CHRTR2 file.");

QString chrtr2_fileBrowseText = 
  startPage::tr ("Use this button to select the input CHRTR2 file");

QString maskText = 
  startPage::tr ("You may enter the mask value to be stored in CHRTR2 cells that have no original input data and "
                 "are marked as land in the 1 second world landmask (based on cleaned up SRTM2 data).");

QString maskLandText = 
  startPage::tr ("Checking this box causes a pre-determined value to be placed in interpolated areas that are less "
                 "than zero (i.e. interpolated land).  Since we're going to mask the land we don't want any faux land "
                 "created by the gridding routine (due to ringing).  The rules for determining where to place the "
                 "pre-determined mask value and what value to use are as follows:<br><br>"
                 "<ul>"
                 "<li>If the grid point is real or hand-drawn/digitized, nothing is done, <b>EVER</b>.</li>"
                 "<li>If the grid point is interpolated and the land mask indicates it is land, it is set to the <b>Mask value</b>.</li>"
                 "<li>If the grid point is interpolated, it is a land value (i.e. less than zero), and the land mask indicates "
                 "it is water, it is filled according to the following rules, based on grid spacing:</li>"
                 "<ul>"
                 "<li>5 minute grid or larger, set to 20 meters.</li>"
                 "<li>2 minute to 5 minute grid, set to 10 meters</li>"
                 "<li>1 minute to 2 minute grid, set to 5 meters</li>"
                 "<li>.5 minute to 1 minute grid, set to 4 meters</li>"
                 "<li>.1 minute grid or smaller, set to 2 meters</li>"
                 "</ul>"
                 "</ul>");
