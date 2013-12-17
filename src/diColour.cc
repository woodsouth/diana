/*
  Diana - A Free Meteorological Visualisation Tool

  $Id: diColour.cc 3903 2012-07-13 11:11:25Z lisbethb $

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <diColour.h>

#include <puTools/miStringFunctions.h>

#include <cstdio>
#include <iomanip>
#include <ostream>

#define MILOGGER_CATEGORY "diana.Colour"
#include <miLogger/miLogging.h>

using namespace std;
using namespace miutil;

map<std::string,Colour> Colour::cmap;
vector<Colour::ColourInfo> Colour::colours;


Colour::Colour(const values& va) : v(va){
}

Colour::Colour(const unsigned long int hexv){
  unsigned long int h= hexv;

  unsigned char a= 255;
  if (h>0xFFFFFF){
    a= (h & 0xFF); h = (h >> 8);
  }
  unsigned char b= (h & 0xFF); h = (h >> 8);
  unsigned char g= (h & 0xFF); h = (h >> 8);
  unsigned char r= (h & 0xFF);

  set(r,g,b,a);
}

Colour::Colour(const std::string& name_)
{
  //  METLIBS_LOG_DEBUG(name_);

  std::string lname= miutil::to_lower(name_);
  vector<std::string> vstr = miutil::split(lname, ":");
  int n = vstr.size();
  if (n<2){
    if(cmap.count(lname))
      memberCopy(cmap[lname]);
    else{
      METLIBS_LOG_DEBUG("Colour:"<<lname<<" unknown");
      set(0,0,0);
      name = "black";
    }
  } else if(n<3) {
    memberCopy(cmap[vstr[0]]);
    v.rgba[alpha]= atoi(vstr[1].c_str());
  } else if(n<4){
    set(atoi(vstr[0].c_str()),atoi(vstr[1].c_str()),atoi(vstr[2].c_str()));
    name = lname;
  } else {
    set(atoi(vstr[0].c_str()),atoi(vstr[1].c_str()),
	atoi(vstr[2].c_str()),atoi(vstr[3].c_str()));
    name = lname;
  }
}

Colour::Colour(const unsigned char r, const unsigned char g,
	       const unsigned char b, const unsigned char a){
  set(r,g,b,a);
  name =miutil::from_number(int(r)) +":";
  name+=miutil::from_number(int(g)) +":";
  name+=miutil::from_number(int(b)) +":";
  name+=miutil::from_number(int(a));
}

// Copy constructor
Colour::Colour(const Colour &rhs){
  // elementwise copy
  memberCopy(rhs);
}

// Destructor
Colour::~Colour(){
}

// Assignment operator
Colour& Colour::operator=(const Colour &rhs){
  if (this != &rhs)
    // elementwise copy
    memberCopy(rhs);

  return *this;
}

// Equality operator
bool Colour::operator==(const Colour &rhs) const{
  return v==rhs.v;
}

void Colour::memberCopy(const Colour& rhs){
  // copy members
  v= rhs.v;
  name= rhs.name;
  colourindex= rhs.colourindex;
}

void Colour::define(const std::string& name_,
		    const unsigned char r, const unsigned char g,
		    const unsigned char b, const unsigned char a){
  Colour c(r,g,b,a);
  std::string lname= miutil::to_lower(name_);
  c.name= lname;
  cmap[lname]= c;
}

void Colour::define(const std::string name_, const values& va){
  Colour c(va);
  std::string lname= miutil::to_lower(name_);
  c.name= lname;
  cmap[lname]= c;
}

void Colour::defineColourFromString(const std::string& rgba_string)
{

  unsigned char r,g,b,a;
  vector<std::string> stokens = miutil::split(rgba_string, ":");
  if (stokens.size()>2 ) {
    r= atoi(stokens[0].c_str());
    g= atoi(stokens[1].c_str());
    b= atoi(stokens[2].c_str());
    if (stokens.size()>3) {
      a= atoi(stokens[3].c_str());
    } else {
      a= 255;
    }
    define(rgba_string,r,g,b,a);
  }

}

// hack: colourindex is platform-dependent
void Colour::setindex(const std::string& name_, const unsigned char index)
{
  std::string lname= miutil::to_lower(name_);
  cmap[lname].colourindex= index;
}

void Colour::addColourInfo(const ColourInfo& ci)
{
  if (not ci.name.empty()) {
    for ( unsigned int q=0; q<colours.size(); q++ )
      if ( colours[q].name == ci.name ){
	colours[q] = ci;
	return;
      }
  }
  colours.push_back(ci);
}

void Colour::readColourMap(const std::string fname){
  FILE *fp;
  char buffer[1024], name_[100];
  std::string lname;
  int r,g,b;
  Colour c;
  if ((fp=fopen(fname.c_str(),"r"))){
    while (fgets(buffer,1023,fp)){
      if (buffer[0]=='#' || buffer[0]=='!') continue;
      sscanf(buffer,"%i %i %i %s\n",&r,&g,&b,name_);
      lname= miutil::to_lower(name_);
      define(lname,r,g,b,255);
    }
    METLIBS_LOG_DEBUG("Found " << cmap.size() << " colours in " << fname);
    fclose(fp);
  }
}


ostream& operator<<(ostream& out, const Colour& rhs){
  return out <<
    " name: "  << rhs.name <<
    " red: "   << setw(3) << setfill('0') << int(rhs.v.rgba[0]) <<
    " green: " << setw(3) << setfill('0') << int(rhs.v.rgba[1]) <<
    " blue: "  << setw(3) << setfill('0') << int(rhs.v.rgba[2]) <<
    " alpha: " << setw(3) << setfill('0') << int(rhs.v.rgba[3]) <<
    " Index: " << int(rhs.colourindex) ;
}
