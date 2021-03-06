/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2017-2018 met.no

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

#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include "diTimeTypes.h"

namespace miutil {

miTime addSec(const miTime& t, int seconds);
miTime addMin(const miTime& t, int minutes);
miTime addHour(const miTime& t, int hours);

plottimes_t::const_iterator nearest(const plottimes_t& times, const miTime& time);

plottimes_t::const_iterator step_time(const plottimes_t& times, const miTime& time, int steps);
plottimes_t::const_iterator step_time(const plottimes_t& times, const miTime& time, const miTime& newTime);

extern const miutil::miTime unix_t0;

//! format a time as string with yyyymmddhhmm
std::string stringFromTime(const miutil::miTime& t, bool addMinutes);

//! get time from a string with yyyymmddhhmm
miutil::miTime timeFromString(const std::string& timeString);

} // namespace miutil

#endif // TIME_UTIL_H
