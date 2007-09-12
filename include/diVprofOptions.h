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
#ifndef VPROFOPTIONS_H
#define VPROFOPTIONS_H

#include <miString.h>
#include <vector>

using namespace std;

/**
   \brief Options/settings/defaults for the Vertical Profile diagram

*/
class VprofOptions
{
public:
  VprofOptions();
  ~VprofOptions();
  void setDefaults();
  void checkValues();
  // read/write setup and log
  vector<miString> writeOptions();
  void readOptions(const vector<miString>& vstr);

//...........protected:
private:
  friend class VprofSetupDialog;
  friend class VprofDiagram;
  friend class VprofPlot;

  bool changed;

  // data to be shown
  bool ptttt;     // t
  bool ptdtd;     // td
  bool pwind;     // wind
  bool pvwind;    // vertical wind
  bool prelhum;   // relative humidty
  bool pducting;  // ducting
  bool pkindex;   // Kindex
  bool pslwind;   // numbers at significant wind levels

  vector<miString> dataColour;
  vector<float>    dataLinewidth;
  vector<float>    windLinewidth;

  bool  windseparate; // separate wind columns when multiple data
  float rvwind;       // range of vertical wind in unit hPa/s (-range to +range)
  float ductingMin;   // min ducting in diagram
  float ductingMax;   // max ducting in diagram

  bool ptext;    // text (station name etc.)
  bool pgeotext; // geographic position in text
  bool temptext; // false=use TEMP  true= use tempname() in text
  int  linetext; // fixed no. of text lines for station and time, 0= not fixed

  //-----------------------------------------------------------------------

  miString backgroundColour;

  int diagramtype; // 0=Amble  1=Exner(pi)  2=Pressure  3=ln(P)
                   // (Amble: ln(P) below 500hPa, P above 500hPa)
  float tangle;    // angle between the vertical and temperatur lines,
                   // 0. - 80. degrees, 45 degrees on Amble diagram
  int pminDiagram;   // min pressure
  int pmaxDiagram;   // max pressure
  int tminDiagram;   // min temperature at 1000 hPa
  int tmaxDiagram;   // max temperature at 1000 hPa
  int trangeDiagram; // temperature range  0=fixed  1=fixed.max-min  2=minimum

  bool pplines;   // pressure lines
  bool pplinesfl; // false=at pressure levels  true=at flight levels
  vector<float> plevels; // pressure
  vector<int>    flightlevels; // flight levels
  vector<float> pflightlevels; // pressure at flight levels
  miString      pColour;
  miString      pLinetype;
  float         pLinewidth1; // thin lines
  float         pLinewidth2; // thick lines

  bool ptlines;   // temperature lines
  int  tStep;     // temp. step
  miString tColour;
  miString tLinetype;
  float    tLinewidth1; // thin lines
  float    tLinewidth2; // thick lines

  bool pdryadiabat;    // dry adiabats
  int  dryadiabatStep; // temperature step (C at 1000hPa)
  miString dryadiabatColour;
  miString dryadiabatLinetype;
  float    dryadiabatLinewidth;

  bool pwetadiabat;    // dry adiabats
  int  wetadiabatStep; // temperature step (C at 1000hPa)
  miString wetadiabatColour;
  miString wetadiabatLinetype;
  float    wetadiabatLinewidth;
  int      wetadiabatPmin;
  int      wetadiabatTmin;

  bool pmixingratio;    // mixing ratio
  int   mixingratioSet; // line set no. (1,2,3,4 available)
  miString mixingratioColour;
  miString mixingratioLinetype;
  float    mixingratioLinewidth;
  int      mixingratioPmin;
  int      mixingratioTmin;
  vector< vector<float> > qtable;

  bool plabelp; // p labels (numbers)
  bool plabelt; // t labels (numbers)
  bool plabelq; // mixing ratio labels (numbers)
  bool pframe;  // frame
  miString frameColour;
  miString frameLinetype;
  float    frameLinewidth;
  miString textColour;

  bool pflevels; // flight levels (numbers/marks on axis only)
  miString flevelsColour;
  miString flevelsLinetype;
  float    flevelsLinewidth1;
  float    flevelsLinewidth2; // thick lines
  bool     plabelflevels; // labels (numbers)

  float rsvaxis;   // relative size vertical axis
  float rstext;    // relative size text
  float rslabels;  // relative size labels (p and t numbers)
  float rswind;    // relative size wind   (width of column)
  float rsvwind;   // relative size vertical wind (width of column)
  float rsrelhum;  // relative size relative humidity (width of column)
  float rsducting; // relative size ducting (width of column)

  miString rangeLinetype;  // for vertical wind, rel.hum. and ducting
  float    rangeLinewidth;

  bool pcotrails; // condensation trail lines
                  // (linjer for vurdering av mulighet for
		  //  kondensstriper fra fly)
  miString cotrailsColour;
  miString cotrailsLinetype;
  float    cotrailsLinewidth;
  int      cotrailsPmin;
  int      cotrailsPmax;

};

#endif

