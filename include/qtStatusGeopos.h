/*
  Diana - A Free Meteorological Visualisation Tool

  $Id$

  Copyright (C) 2006 met.no

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
#ifndef _qtStatusGeopos_h
#define _qtStatusGeopos_h

#include <qwidget.h>
#include <miString.h>

class QLabel;

using namespace std; 


/**
   \brief Geoposition in status bar
   
   Widget in status bar for displaying the geographical position of the mouse pointer

*/

class StatusGeopos : public QWidget {
  Q_OBJECT
private:
  bool geographicMode;
  bool decimalMode;
  QLabel *sylabel;
  QLabel *sxlabel;
public:
  StatusGeopos(QWidget* parent = 0, const char* name = 0);
  /// toggle between geo/xy display
  void changeMode();
  void changeLatLonMode();
  bool getGeographicMode() { return geographicMode; }
  
public slots:
void setPosition(float, float);

protected:
  QLabel *latlabel;
  QLabel *lonlabel;
};

#endif
