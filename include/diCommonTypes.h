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
#ifndef _diCommonTypes_h
#define _diCommonTypes_h

#include <miTime.h>
#include <miString.h>
#include <vector>
#include <diColour.h>

using namespace std;


/**
   \brief GUI slider data
*/
struct SliderValues{
   int minValue;
   int maxValue;
   int value;
   float scale;
};

//--------------------------------------------------
// image
//--------------------------------------------------

/**
   \brief GUI data for one geo image source
*/
struct SatFileInfo{
  miString name;
  miTime time;
  miClock clock;
  int day;
  bool opened;
  vector<miString> channel;
  miString default_channel;
  bool palette; //palette or rgb file
  vector <Colour> col;    //vector of colours used
};

/**
   \brief GUI data for geo images
*/
struct SatDialogInfo{

  /// image file info
  struct File {
    miString name;            ///< filename
    vector<miString> channel; ///< channels available
    miString default_channel; ///< default channel
  };

  /// main image info
  struct Image{
    miString name;           ///< main image name
    vector<File> file;       ///< files available
    miString default_file;   ///< default filename
  };

  vector<Image> image;   ///< defined Images
  SliderValues cut;      ///< rgb cutoff value for histogram stretching
  SliderValues alphacut;  ///< rgb cutoff value for alpha blending
  SliderValues alpha;     ///< alpha blending value
  SliderValues timediff; ///< max time difference
};


//--------------------------------------------------
//info about object files
//--------------------------------------------------

/**
   \brief GUI data for object file
*/
struct ObjFileInfo{
  miString name; ///< name of the file
  miTime time;   ///< time of the file
};


//--------------------------------------------------
// Map structures
//--------------------------------------------------

/**
   \brief GUI data for one map file
*/
struct MapFileInfo {
  miString fname;     ///< filename
  double sizelimit; ///< viewsize limit in km's
};

/**
   \brief GUI data for one map element
*/
struct MapElementOption {
  bool ison;         ///< element is on
  miString linecolour; ///< linecolour
  miString fillcolour; ///< fillcolour
  miString linetype;   ///< linetype
  miString linewidth;  ///< linewidth
  int zorder;        ///< z-position on map
  float density;     ///< density in degrees (latlon)
};

/**
   \brief GUI data for one map
*/
struct MapInfo {
  miString name;             ///< name of map
  miString type;             ///< type of mapsource(s)
  bool logok;                ///< ok to log
  vector<MapFileInfo> mapfiles; ///< the file(s)
  MapElementOption contour;  ///< contourline options
  MapElementOption land;     ///< filled land options
  MapElementOption latlon;   ///< latlon-lines options
  MapElementOption frame;    ///< area-frame options
};

/**
   \brief GUI data for all maps
*/
struct MapDialogInfo {
  vector<miString> areas; ///< all defined areas (names)
  miString default_area;  ///< default area-name
  vector<MapInfo> maps;   ///< all defined maps
  vector<miString> default_maps; ///< list of default maps (names)
  miString backcolour;    ///< background colour
};


//--------------------------------------------------
// Observation structures
//--------------------------------------------------

/**
   \brief GUI data for all observations
*/
struct ObsDialogInfo{

  /// list of prioritized stations
  struct PriorityList{
    miString file;
    miString name;
  };

  /// list of plotting criterias
  struct CriteriaList{
    miString name;
    vector<miString> criteria;
  };

  /// observation data type
  struct DataType {
    miString name;
    vector<bool> active;///< same length as button
  };

  /// data button info for observation dialogue
  struct Button {
    miString name;
    miString tooltip;
    int high,low;
    bool Default;
  };

  /// observation plot type
  struct PlotType {
    miString name;
    vector<DataType> datatype;
    vector<Button> button;
    miString misc;
    vector<int> pressureLevels; ///<Only used for PlotType "Pressure levels"
    vector<CriteriaList> criteriaList;
  };

  vector<PlotType> plottype;
  vector<PriorityList> priority;
  SliderValues density, size, timediff;
  miString defValues;
};


//--------------------------------------------------
// Objects/Edit
//--------------------------------------------------

/**
   \brief data for one edit tool
*/
struct editToolInfo {
  miString name;
  int index;
  miString colour;
};

/**
   \brief data for one edit mode (field-editing, object-drawing)
*/
struct editModeInfo {
  miString editmode;
  vector<editToolInfo> edittools;
};

/**
   \brief data for one map mode (normal, editing)
*/
struct mapModeInfo {
  miString mapmode;
  vector<editModeInfo> editmodeinfo;
};

/**
   \brief data for all edititing functions
*/
struct EditDialogInfo {
  vector<mapModeInfo> mapmodeinfo;
};


/**
   \brief metadata on one map area
*/
struct selectArea{
  miString name; ///<name of area ie. VV, Hordaland etc.
  bool selected; ///<
  int id;        ///<id of group(areaObjects)
};

//--------------------------------------------------

/**
   \brief geo image data value in one position
*/
struct SatValues{
  miString channel;
  float value; ///<rgb
  miString text; ///<palette
};


/**
   \brief quick menu type
*/
struct QuickMenuDefs {
  miString filename;
};


/**
   \brief metadata on one data layer (on map)
*/
struct PlotElement {
  miString type;
  miString str;
  miString icon;
  bool enabled;
  PlotElement():enabled(false){}
  PlotElement(miString t,miString s,miString i,bool e):
    type(t),str(s),icon(i),enabled(e){}
};

/**
   \brief text information sources
*/
struct InfoFile {
  miString name;    ///< name of source
  miString filename;///< name of file
  miString doctype; ///< documenttype: auto,xml,html,text
  miString fonttype;///< fonttype: auto, fixed
};

#endif
