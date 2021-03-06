/*
 Diana - A Free Meteorological Visualisation Tool

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
#ifndef _diMapManager_h
#define _diMapManager_h

/*
 \file

 \brief Manager of maps and predefined areas/projections
 */

#include "diPlotOptions.h"
#include "diMapInfo.h"
#include "diField/diArea.h"
#include "util/diKeyValue.h"

/**
 \brief Manager of maps and predefined areas/projections

 Manages datafiles and setup information for maps
*/
class MapManager {
private:
  static std::vector<MapInfo> mapfiles;

public:
  MapManager();

  /// parse the maps section in the setup file
  bool parseSetup();

  /// get list of defined maps
  const std::vector<MapInfo>& getMapInfo();
  /// get information on one specific map
  bool getMapInfoByName(const std::string&, MapInfo&);

  bool fillMapInfo(const miutil::KeyValue_v&, MapInfo&);

  /// extract plot information from string
  bool fillMapInfo(const miutil::KeyValue_v&, MapInfo&, PlotOptions& contopts,
      PlotOptions& landopts, PlotOptions& lonopts, PlotOptions& latopts,
      PlotOptions& ffopts);
  /// make string representation of one MapInfo
  miutil::KeyValue_v MapInfo2str(const MapInfo&);
  /// make string representation of lat/lon/frame/background info
  miutil::KeyValue_v MapExtra2str(const MapInfo&);
  /// get all defined maps and areas for the GUI
  MapDialogInfo getMapDialogInfo();
};

#endif
