/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Topology/XShape.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/LabelBlock.hpp"
#include "Units.hpp"
#include "UtilsText.hpp"

#include <tchar.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

static char *
import_label(const char *src)
{
  if (src == NULL || strcmp(src, "UNK") == 0 ||
      strcmp(src, "RAILWAY STATION") == 0 ||
      strcmp(src, "RAILROAD STATION") == 0)
    return NULL;

  if (ispunct(src[0])) {
    double value = strtod(src + 1, NULL);
    value = Units::ToUserUnit(value, Units::AltitudeUnit);

    char buffer[32];
    if (value > 999)
      sprintf(buffer, "%.1f", (value / 1000));
    else
      sprintf(buffer, "%d", (int)value);

    return strdup(buffer);
  }

  return strdup(src);
}

XShape::XShape(shapefileObj *shpfile, int i, int label_field)
  :label(NULL), hide(false)
{
  msInitShape(&shape);
  msSHPReadShape(shpfile->hSHP, i, &shape);

#ifdef RADIANS
  for (int tt = 0; tt < shape.numlines; ++tt) {
    for (int jj = 0; jj < shape.line[tt].numpoints; ++jj) {
      shape.line[tt].point[jj].x *= DEG_TO_RAD;
      shape.line[tt].point[jj].y *= DEG_TO_RAD;
    }
  }
#endif

  if (label_field >= 0) {
    const char *src = msDBFReadStringAttribute(shpfile->hDBF, i, label_field);
    label = import_label(src);
    if (label == NULL)
      hide = true;
  }
}

XShape::~XShape()
{
  free(label);
  msFreeShape(&shape);
}

void
XShape::renderSpecial(Canvas &canvas, LabelBlock &label_block,
                      int x, int y) const
{
  if (!label)
    return;

  TCHAR Temp[100];
  ConvertCToT(Temp, label);

  SIZE tsize = canvas.text_size(Temp);

  x += 2;
  y += 2;

  RECT brect;
  brect.left = x;
  brect.right = brect.left + tsize.cx;
  brect.top = y;
  brect.bottom = brect.top + tsize.cy;

  if (!label_block.check(brect))
    return;

  canvas.background_transparent();
  canvas.set_text_color(Color(0x20, 0x20, 0x20));
  canvas.text(x, y, Temp);
  canvas.background_opaque();
}
