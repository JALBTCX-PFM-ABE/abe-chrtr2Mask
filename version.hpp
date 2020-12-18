
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




#ifndef VERSION

#define     VERSION     "PFM Software - chrtr2Mask V2.08 - 08/27/16"

#endif

/*! <pre>

  Version 1.00
  Jan C. Depner
  12/28/10

  First working version.


  Version 1.01
  Jan C. Depner
  01/06/11

  Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


  Version 1.02
  Jan C. Depner
  01/13/11

  Added ability to use old WVS land mask.


  Version 1.10
  Jan C. Depner
  01/18/11

  Added edge feathering.  This was taken almost verbatim from fingrd.  The only difference is that I refuse to replace
  real, surveyed data with fake land values.


  Version 1.20
  Jan C. Depner
  01/24/11

  Finally got it right.  Removed edge feathering, force value, WVS, SRTM_TOPO.  Added zero set for NGA coastline.  CHRTR2_LAND_MASK
  data is now ignored by CHRTR2 programs (other than to show it).  Added final regrid at end of masking.


  Version 1.21
  Jan C. Depner
  06/27/11

  Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


  Version 1.22
  Jan C. Depner
  07/22/11

  Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
  last used directory are in the sidebar URL list of QFileDialogs.


  Version 1.23
  Jan C. Depner
  11/30/11

  Converted .xpm icons to .png icons.


  Version 2.0
  Stacy Johnson	
  06/20/2012

  Adjusted program for move chrtr2 to grid orientation


  Version 2.01
  Stacy Johnson	
  08/22/2012

  Additional chrtr2 modifications for grid orientation


  Version 2.02
  Stacy Johnson	
  11/13/2012

  Per request from Paul Marin the program no longer automaticaly remisps the data.
  Per request from Kristina Amacker and apporval from Paul Marin, program assigns -1 (land) and 1 (water) to all points. 


  Version 2.03
  Stacy Johnson	
  02/14/2013

  Per Paul Marin, changed 0.0 (RIVERS/ETC) from SRTM into -1 (LAND).


  Version 2.04
  Stacy Johnson	
  08/14/2013

  Corrected mislabeled areas outside of landmask.


  Version 2.05
  Jan C. Depner (PFM Software)
  12/09/13

  Switched to using .ini file in $HOME (Linux) or $USERPROFILE (Windows) in the ABE.config directory.  Now
  the applications qsettings will not end up in unknown places like ~/.config/navo.navy.mil/blah_blah_blah on
  Linux or, in the registry (shudder) on Windows.


  Version 2.06
  Jan C. Depner (PFM Software)
  07/01/14

  - Replaced all of the old, borrowed icons with new, public domain icons.  Mostly from the Tango set
    but a few from flavour-extended and 32pxmania.


    Version 2.07
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 2.08
    Jan C. Depner (PFM Software)
    08/27/16

    - Now uses the same font as all other ABE GUI apps.  Font can only be changed in pfmView Preferences.

</pre>*/
