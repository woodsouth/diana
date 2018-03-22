#include "diana_config.h"


#include "diObsReaderRoad.h"

#include "diObsDialogInfo.h"

// includes for road specific implementation
#include <diObsRoad.h>

#include <puTools/miStringFunctions.h>

#define MILOGGER_CATEGORY "diana.ObsReaderRoad"
#include <miLogger/miLogging.h>

using namespace miutil;
using namespace std;

ObsReaderRoad::ObsReaderRoad()
{
  setTimeRange(-180, 180);
}

bool ObsReaderRoad::configure(const std::string& key, const std::string& value)
{
  if (key == "roadobs" || key == "archive_roadobs") {
    addPattern(value, key == "archive_roadobs");
  } else if (key == "headerfile") {
    headerfile = value;
  } else if (key == "databasefile") {
    databasefile = value;
  } else if (key == "stationfile") {
    stationfile = value;
  } else if (key == "daysback") {
    daysback = miutil::to_int(value);
  } else {
    return ObsReaderFile::configure(key, value);
  }
  return true;
}

void ObsReaderRoad::getDataFromFile(const FileInfo& fi, ObsDataRequest_cp request, ObsDataResult_p result)
{
#ifdef ROADOBS
  METLIBS_LOG_DEBUG(fi.filename << ", " << databasefile << ", " << stationfile << ", " << headerfile << ", " << fi.time);
  // The constructor with last argument false init the internal datastructures, but reads no data.
  ObsRoad obsRoad(fi.filename, databasefile, stationfile, headerfile, fi.time, request, false);
  // readData reads the data from the SMHI database.
  std::vector<ObsData> obsdata;
  obsRoad.readData(obsdata,request);
  for (ObsData& obs : obsdata)
    obs.dataType = dataType();
  result->add(obsdata);
#endif
}

std::vector<ObsDialogInfo::Par> ObsReaderRoad::getParameters()
{
#ifdef ROADOBS
  // The road format must have a header file, defined in prod
  // This file, defines the parameters as well as the mapping
  // between diana and road parameter space.

  std::string filename; // just dummy here
  miTime filetime;      // just dummy here
  ObsDataRequest_cp request; // just dummy here
  
  METLIBS_LOG_DEBUG(databasefile << ", " << stationfile << ", " << headerfile);
  
  ObsRoad obsRoad = ObsRoad(filename, databasefile, stationfile, headerfile, filetime, request, false);
  std::vector<ObsDialogInfo::Par> parameters;
  
  for (size_t c = 0; c < obsRoad.columnCount(); c++) {
    ObsDialogInfo::Par pt(obsRoad.columnName(c),ObsDialogInfo::ParType::pt_std,0,0,obsRoad.columnTooltip(c),0,1);
    parameters.push_back(pt);
  }

  
  return parameters;
#else
  return ObsReader::getParameters();
#endif
}
