/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2019 met.no

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
#ifndef _diWeatherObjects_h
#define _diWeatherObjects_h

#include "diAreaBorder.h"

#include "diPlotCommand.h"

#include "diField/diArea.h"

#include <vector>

/**
   \brief WeatherObjects to be edited or displayed
*/
class WeatherObjects {
public:
  WeatherObjects();
  virtual ~WeatherObjects();

  /// the weather objects to plot
  std::vector<ObjectPlot*> objects;

  /// clears all variables
  void clear();

  virtual void init(){}

  /// returns true of no objects or labels
  bool empty() const;

  /// plot all weather objects
  void plot(DiGLPainter* gl, PlotOrder porder);

  /// check if objectplots are enabled
  bool isEnabled() const;

  /// enable/disable objectplots
  void enable(const bool b);

  /// set prefix for object files
  void setPrefix(const std::string& p)
    { prefix=p; }

  /// sets the object time
  void setTime(const miutil::miTime& t)
    { itsTime=t; }

  /// gets the object time
  const miutil::miTime& getTime() const;

  /// returns number of object plots
  int getSize()
    { return objects.size(); }

  /// return objects area
  const Area& getArea() const
    { return itsArea; }

  /// set objects area
  void setArea(const Area & area)
    { itsArea = area; }

  /// updates the bound box
  void updateObjects();

  /// request later reprojection to newArea
  void changeProjection(const Area& newArea, const Rectangle& plotSize);

  /// force projection to newArea projection
  bool switchProjection(const Area& newArea);

  bool convertFromProjection(const Area& pointProjection, int n, float* x, float* y);
  bool convertToProjection(const Area& pointProjection, int n, float* x, float* y);

  /// read file with comments
  bool readEditCommentFile(const std::string fn);

  /// returns objects' old comments
  std::string readComments();

  ///return oldLabels from object file
  const PlotCommand_cpv& getObjectLabels();

  /// return new edited labels
  const PlotCommand_cpv& getEditLabels();

  /// read file with weather objects and change projection to newArea
  bool readEditDrawFile(const std::string& filename);

  /// read  string with weather objects and change projection to newArea
  bool readEditDrawString(const std::string& inputString, bool replace = false);

  /// read file with area borders (for combining analyses)
  bool readAreaBorders(const std::string& fn);

  /// write file with area borders (for combining analyses)
  bool writeAreaBorders(const std::string&);

  /// writes string with edited objects
  std::string writeEditDrawString(const miutil::miTime&);

  /// sets which object types should be plotted
  void setSelectedObjectTypes(const std::string& t) { useobject = decodeTypeString(t); }

  /// sets which object types should be plotted
  void setSelectedObjectTypes(const std::vector<std::string>& types) { useobject = decodeTypeString(types); }

  /// returns number of object of this type
  int objectCount(int type);

  /// add an object
  void addObject(ObjectPlot* object, bool replace=false);

  /// remove an object
  std::vector<ObjectPlot*>::iterator removeObject(std::vector<ObjectPlot*>::iterator);

 /// the file the objects are read from
  std::string filename;

  /// decode string with types of objects to plot
  static std::map<std::string, bool> decodeTypeString(const std::string&);
  static std::map<std::string, bool> decodeTypeString(const std::vector<std::string>& types);

  /// x,y for copied objects
  float xcopy,ycopy;

protected:
  static const Area& getGeoArea();

private:
  std::map<std::string,bool> useobject;
  bool enabled;

protected:
  std::string prefix;               //VA,VV,VNN...
  Area itsArea;                  // current object area
  Area mapArea;                  // map object area, to be used for next plot

  miutil::miTime itsTime;                //plot time
  std::string itsOldComments;       // the comment string to edit
  PlotCommand_cpv itsLabels;            //edited labels
  PlotCommand_cpv itsOldLabels;         //labels read in from object file
};

#endif
