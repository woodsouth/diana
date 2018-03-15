/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2017 met.no

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

#include "RasterFileSink.h"

RasterFileSink::RasterFileSink(const QSize& size, const QString& filename)
    : image_(size, QImage::Format_ARGB32_Premultiplied)
    , filename_(filename)
{
}

bool RasterFileSink::isPrinting()
{
  return false;
}

bool RasterFileSink::beginPage()
{
  image_.fill(Qt::transparent);
  painter_.begin(&image_);
  return true;
}

QPainter& RasterFileSink::paintPage()
{
  return painter_;
}

bool RasterFileSink::endPage()
{
  painter_.end();
  return true;
}

bool RasterFileSink::finish()
{
  if (!filename_.isEmpty())
    return saveTo(filename_);
  else
    return true;
}

bool RasterFileSink::saveTo(const QString& filename)
{
  if (!filename.isEmpty())
    return image_.save(filename);
  else
    return false;
}