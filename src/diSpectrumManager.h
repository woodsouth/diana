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
#ifndef SPECTRUMMANAGER_H
#define SPECTRUMMANAGER_H

#include "diCommonTypes.h"
#include "diTimeTypes.h"
#include "vcross_v2/VcrossSetup.h"

#include <puTools/miTime.h>

#include <vector>
#include <map>
#include <set>

class DiGLPainter;
class SpectrumOptions;
class SpectrumFile;
class SpectrumData;
class SpectrumPlot;

class QSize;

/**
   \brief Managing Wave Spectrum data sources and plotting
*/
class SpectrumManager
{
public:
  struct SelectedModel
  {
    std::string model;
    std::string reftime;
  };

private:
  struct StationPos {
    float latitude;
    float longitude;
    std::string obs;
  };

  // map<model,filename>
  std::map<std::string,std::string> filenames;
  std::map<std::string,std::string> filetypes;
  std::map<std::string,std::string> filesetup;
  vcross::Setup_p setup;

  // for use in dialog (unique lists in setup order)
  std::vector<std::string> dialogModelNames;
  std::vector<std::string> dialogFileNames;

  std::unique_ptr<SpectrumOptions> spopt;
  std::vector<SpectrumFile*> spfile;
  std::vector<SpectrumData*> spdata;

  std::vector<std::string> nameList;
  std::vector<float> latitudeList;
  std::vector<float> longitudeList;
  plottimes_t timeList;

  std::vector<SelectedModel> selectedModels;

  int plotw, ploth;

  std::string plotStation;
  miutil::miTime   plotTime;

  int realizationCount;
  int realization;

  bool dataChange;
  std::vector<SpectrumPlot*> spectrumplots;

  std::map<std::string,std::string> menuConst;

  std::string getDefaultModel();
  bool initSpectrumFile(const SelectedModel& selectedModel);
  void initStations();
  void initTimes();
  void preparePlot();

public:
  SpectrumManager();
  ~SpectrumManager();

  void parseSetup();
  SpectrumOptions* getOptions() { return spopt.get(); }
  void setPlotWindow(const QSize& size);

  //routines from controller
  std::vector<std::string> getLineThickness();

  void setModel();
  void setRealization(int r);
  void setStation(const std::string& station);
  void setTime(const miutil::miTime& time);
  void setStation(int step);
  void setTime(int step, int dir);
  void setTime(int step);

  const miutil::miTime& getTime(){return plotTime;}
  const std::string& getStation() { return plotStation; }
  const std::vector<std::string>& getStationList() { return nameList; }
  const std::vector<float>& getLatitudes() { return latitudeList; }
  const std::vector<float>& getLongitudes() { return longitudeList; }
  const plottimes_t& getTimeList() { return timeList; }

  std::vector<std::string> getModelNames();
  std::vector<std::string> getModelFiles();
  std::vector <std::string> getReferencetimes(const std::string& model);
  void setSelectedModels(const std::vector<std::string>& models);
  const std::vector<SelectedModel>& getSelectedModels() const { return selectedModels; }

  bool plot(DiGLPainter* gl);
  void mainWindowTimeChanged(const miutil::miTime& time);
  std::string getAnnotationString();

  void setMenuConst(const std::map<std::string,std::string>& mc)
    { menuConst = mc;}

  std::vector<std::string> writeLog();
  void readLog(const std::vector<std::string>& vstr,
      const std::string& thisVersion, const std::string& logVersion);
};

#endif
