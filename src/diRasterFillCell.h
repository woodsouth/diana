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

#ifndef diRasterFillCell_h
#define diRasterFillCell_h

#include "diRasterGrid.h"

class PlotOptions;
class Colour;

class RasterFillCell : public RasterGrid
{
public:
  RasterFillCell(const PlotArea& pa, Field* f, PlotOptions& po);
  void colorizePixel(QRgb& pixel, const diutil::PointI& i) override;

  const Colour* colourForValue(float value) const;

private:
  PlotOptions& poptions;
};

#endif // diRasterFillCell_h
