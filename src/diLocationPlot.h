/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2015 met.no

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
#ifndef LocationPlot_h
#define LocationPlot_h

#include "diPlot.h"
#include "diLocationData.h"

/**
   \brief Plot pickable Locations on the map

   Contains data for a set of locations.
   At present only used to display lines of Vertical Crossections
   on the map.
*/
class LocationPlot : public Plot
{
public:
  LocationPlot();
  ~LocationPlot() override;

  void plot(DiGLPainter* gl, PlotOrder zorder) override;
  void changeProjection(const Area& mapArea, const Rectangle& plotSize) override;
  std::string getEnabledStateKey() const override;
  void getAnnotation(std::string& str, Colour& col) const override;

  bool setData(const LocationData& locationdata);

  void setSelected(const std::string& name) { selectedName = name; }

  void setVisible(bool vis) { visible = vis; }
  bool isVisible() const { return visible; }
  const std::string& getName() const { return locdata.name; }
  std::string find(int x, int y);

private:
  void drawLineOrPoint(DiGLPainter* gl, int l);
  void switchProjection(const Area& mapArea);

private:
  struct InternalLocationInfo {
    int   beginpos;
    int   endpos;
    float xmin,xmax;
    float ymin,ymax;
    float dmax;
  };

  bool visible;

  LocationData locdata;

  std::vector<InternalLocationInfo> locinfo;

  std::string selectedName;

  Area  posArea;
  float sizeOfCross_;

  int   numPos;
  float *px;
  float *py;

  void cleanup();

};

#endif // LocationPlot_h
