
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



#ifndef CHRTR2MASKDEF_H
#define CHRTR2MASKDEF_H

#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "nvutility.h"
#include "nvutility.hpp"
#include "read_srtm_topo.h"
#include "chrtr2.h"
#include "misp.h"


#define         NUMSHADES           40
#define         NUMHUES             255
#define         SAMPLE_HEIGHT       200
#define         SAMPLE_WIDTH        130


typedef struct
{
  int32_t       window_x;
  int32_t       window_y;
  int32_t       window_width;
  int32_t       window_height;
  uint8_t       dateline;
  double        mask;
  uint8_t       mask_interp_land;
  int32_t       tasks;
  float         min_z;
  float         max_z;
  QString       input_dir;
  QFont         font;                       //  Font used for all ABE GUI applications
} OPTIONS;



typedef struct
{
  QGroupBox           *mbox;
  QProgressBar        *mbar;
} RUN_PROGRESS;



#endif
