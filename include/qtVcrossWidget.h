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
#ifndef VCROSSWIDGET_H
#define VCROSSWIDGET_H

#include <iostream>
#include <qgl.h>
#include <diColour.h>
#include <diPrintOptions.h>

using namespace std;

class VcrossManager;


/**
   \brief The OpenGL widget for Vertical Crossections

   Handles widget paint/redraw events.
   Receives mouse and keybord events and initiates actions.
*/
class VcrossWidget : public QGLWidget
{
  Q_OBJECT

public:
  VcrossWidget(VcrossManager *vcm, const QGLFormat fmt,
	       QWidget* parent = 0, const char* name = 0 );
  ~VcrossWidget();

  void initializeGL();
  void paintGL();
  void resizeGL( int w, int h );
  void enableTimeGraph(bool on);

  bool saveRasterImage(const miString fname,
  		       const miString format,
		       const int quality = -1);

  void startHardcopy(const printOptions& po);
  void endHardcopy();

protected:

private:
  VcrossManager *vcrossm;

  int plotw, ploth;   // size of widget (pixels)
  GLuint *fbuffer;    // fake overlay buffer (only while zooming)
  float glx1,gly1,glx2,gly2; // overlay buffer area
  Colour rubberbandColour;   // contrast to background

  bool savebackground; // while zooming
  bool dorubberband;   // while zooming
  bool dopanning;

  int arrowKeyDirection;
  int firstx, firsty, mousex, mousey;

  bool timeGraph;
  bool startTimeGraph;

  void keyPressEvent(QKeyEvent *me);
  void mousePressEvent(QMouseEvent* me);
  void mouseMoveEvent(QMouseEvent* me);
  void mouseReleaseEvent(QMouseEvent* me);

signals:
  void timeChanged(int);
  void crossectionChanged(int);

};


#endif
