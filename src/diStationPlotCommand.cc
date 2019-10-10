/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2018 met.no

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

#include "diStationPlotCommand.h"

#include <puTools/miStringFunctions.h>
#include <boost/algorithm/string/join.hpp>
#include <sstream>

#define MILOGGER_CATEGORY "diana.StationPlotCommand"
#include <miLogger/miLogging.h>

static const std::string STATION = "STATION";

StationPlotCommand::StationPlotCommand()
{
}

const std::string& StationPlotCommand::commandKey() const
{
  return STATION;
}

std::string StationPlotCommand::toString() const
{
  std::ostringstream s;
  s << STATION
    << ' ' << name // may contain spaces -- no quotes here!
    << ' ' << url
    << ' ' << select
	<< ' ' << show_names;
  return s.str();
}

// static
StationPlotCommand_cp StationPlotCommand::fromString(const std::string& line)
{
  StationPlotCommand_p cmd = std::make_shared<StationPlotCommand>();
  std::vector<std::string> pieces = miutil::split(line, " ");
  
  if (pieces.size() != 4)
  {
    METLIBS_LOG_ERROR("StationPlotCommand must contain 4 strings");
	return cmd;
  }
  
  cmd->show_names = pieces.back();
  pieces.pop_back();

  cmd->select = pieces.back();
  pieces.pop_back();

  cmd->url = pieces.back();
  pieces.pop_back();

  cmd->name = boost::algorithm::join(pieces, " ");
  return cmd;
}
