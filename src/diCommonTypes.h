/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DIANA_DICOMMONTYPES_H
#define DIANA_DICOMMONTYPES_H

#include <puDatatypes/miCoordinates.h>

#include <string>
#include <vector>

/**
   \brief text information sources
*/
struct InfoFile {
  std::string name;    ///< name of source
  std::string filename;///< name of file
  std::string doctype; ///< documenttype: auto,xml,html,text
  std::string fonttype;///< fonttype: auto, fixed
};

/**
   \brief information about polylines
 */
struct PolyLineInfo {
  int id;
  std::string name;
  std::vector<LonLat> points;
  PolyLineInfo(int i, std::string n, std::vector<LonLat> p)
    : id(i), name(n), points(p) {}
};

#endif // DIANA_DICOMMONTYPES_H
