/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2014 met.no

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

#include "diImageGallery.h"

#include "diGlUtilities.h"
#include "diImageIO.h"
#include "diUtilities.h"
#include "miSetupParser.h"

#include <puTools/miStringFunctions.h>

#include <fstream>

#define MILOGGER_CATEGORY "diana.ImageGallery"
#include <miLogger/miLogging.h>

using namespace::miutil;
using namespace std;

//int ImageGallery::numimages= 0;
//ImageGallery::image ImageGallery::Images[ImageGallery::maximages];
map<std::string,ImageGallery::image> ImageGallery::Images;
map<std::string,ImageGallery::pattern> ImageGallery::Patterns;
map<int, vector<std::string> > ImageGallery::Type;


ImageGallery::image::image()
  :alpha(true)
  ,width(0)
  ,height(0)
  ,data(0)
  ,type(basic)
  , read_error(false)
{
}

ImageGallery::image::~image()
{
  erase();
}

void ImageGallery::image::erase()
{
  name.clear();
  width=  0;
  height= 0;
  alpha= true;
  delete[] data;
  data= 0;
  type= basic;
  read_error= false;
}

ImageGallery::pattern::pattern()
  :pattern_data(0)
  ,read_error(false)
{
}

ImageGallery::pattern::~pattern()
{
  erase();
}

void ImageGallery::pattern::erase()
{
  name.clear();
  delete[] pattern_data;
  pattern_data= 0;
  read_error= false;
}

ImageGallery::Line::Line()
  : width(1)
  , fill(false)
  , circle(false)
  , radius(-1)
{
}

void ImageGallery::Line::invalidate()
{
  points.clear();
  circle = false;
}

bool ImageGallery::Line::valid() const
{
  if (circle) {
    return (radius > 0);
  } else {
    // line, maybe filled
    return (width > 0 && points.size() >= (2 + (fill ? 1 : 0)));
  }
}

// -------------- IMAGEGALLERY -------------------------------

ImageGallery::ImageGallery()
{
}


void ImageGallery::clear()
{
  map<std::string,image>::iterator p= Images.begin();
  for( ; p!=Images.end(); p++){
    p->second.erase();
    Images.erase(p->first);
  }
  map<std::string,pattern>::iterator q= Patterns.begin();
  for( ; q!=Patterns.end(); q++){
    q->second.erase();
    Patterns.erase(q->first);
  }
}


void ImageGallery::addImageName(const std::string& filename, int type)
{
  int n = filename.find_last_of("/");
  int m = filename.find_last_of(".");
  std::string name = filename.substr(n+1,m-n-1);

  if(type == fillpattern){
    if(Patterns.count(name)>0)
      Patterns[name].erase();

    Patterns[name].name=name;
    Patterns[name].filename=filename;
    Type[type].push_back(name);
    return;
  }

  // images, markers ..
  if(Images.count(name)>0)
    Images[name].erase();

  Images[name].name=name;
  Images[name].filename=filename;
  Images[name].type=type;
  Type[type].push_back(name);

  //marker
  if(type == marker){
    std::string markerFilename = filename;
    if(miutil::contains(filename, "png"))
      miutil::replace(markerFilename, "png","txt");
    else if(miutil::contains(filename, "xpm"))
      miutil::replace(markerFilename, "xpm","txt");

    const diutil::string_v matches = diutil::glob(markerFilename);
    if (matches.size() == 1) {
      Images[name].markerFilename = matches.front();
    } else {
      Images[name].type = basic;
    }
  }
}

bool ImageGallery::readImage(const std::string& name)
{
  if(Images[name].data != 0)
    return true; //Image ok

  // if read_error == true, we have tried and failed before
  // Should we try again anyway?
  if (Images[name].read_error)
    return false;

  //read image from file
  imageIO::Image_data img(Images[name].filename);
  img.pattern = false;
  if( !imageIO::read_image(img)){
    if (!Images[name].read_error)
      METLIBS_LOG_ERROR("ImageGallery::readImage ERROR couldn't read image:"<<name);
    Images[name].read_error= true;
    return false;
  }

  return addImage(name,img.width,img.height,
      img.data,img.nchannels>3);
}

bool ImageGallery::readPattern(const std::string& name)
{
  if( Patterns[name].pattern_data!=0)
    return true; //Pattern ok

  // if read_error == true, we have tried and failed before
  // Should we try again anyway?
  if (Patterns[name].read_error)
    return false;

  //read image from file
  imageIO::Image_data img(Patterns[name].filename);
  img.pattern = true;
  if( !imageIO::read_image(img)){
    if (!Patterns[name].read_error)
      METLIBS_LOG_ERROR("ImageGallery::readImage ERROR couldn't read image:"<<name);
    Patterns[name].read_error= true;
    return false;
  }

  return addPattern(name,img.data);

}

bool ImageGallery::addImage(const image& im)
{
  return addImage(im.name,im.width,im.height,im.data,im.alpha);
}

bool ImageGallery::addImage(const std::string& name,
    const int w,
    const int h,
    const unsigned char* d,
    const bool a)
{

  int size= w*h;

  if (name.empty()) {
    METLIBS_LOG_ERROR("ImageGallery::addImage ERROR trying to add image with no name");
    return false;
  }
  if (size == 0){
    METLIBS_LOG_ERROR("ImageGallery::addImage ERROR trying to add image with zero width/height:"
    << name);
    return false;
  }
  if (d==0){
    METLIBS_LOG_ERROR("ImageGallery::addImage ERROR trying to add image with no imagedata:"
    << name);
    return false;
  }

  // add image
  Images[name].name=   name;
  Images[name].width=  w;
  Images[name].height= h;
  Images[name].alpha=  a;
  int fsize = size * (a ? 4 : 3);
  Images[name].data= new unsigned char [fsize];
  for (int j=0; j<fsize; j++)
    Images[name].data[j]= d[j];

  if(Images[name].type==marker)
    if(!readFile(name,Images[name].markerFilename))
      Images[name].type = basic;

  return true;
}

bool ImageGallery::addPattern(const std::string& name,
    const unsigned char* d)
{


  if ((name.empty())){
    METLIBS_LOG_ERROR("ImageGallery::addPattern ERROR trying to add image with no name");
    return false;
  }
  if (d==0){
    METLIBS_LOG_ERROR("ImageGallery::addPattern ERROR trying to add image with no data:"
    << name);
    return false;
  }

  // add pattern
  Patterns[name].name=   name;
  Patterns[name].pattern_data= new unsigned char [128];
  for (int j=0; j<128; j++)
    Patterns[name].pattern_data[j]= (DiGLPainter::GLubyte)d[j];

  return true;
}

float ImageGallery::width_(const std::string& name)
{
  float w= 0.0;
  if (!Images.count(name)){
    METLIBS_LOG_ERROR("ERROR image not found: '" << name << "'");
  } else {
    readImage(name);
    w= Images[name].width;
  }
  return w;
}

float ImageGallery::height_(const std::string& name)
{
  float h= 0.0;
  if (!Images.count(name)){
    METLIBS_LOG_ERROR("ERROR image not found: '" << name << "'");
  } else {
    readImage(name);
    h= Images[name].height;
  }
  return h;
}

int ImageGallery::widthp(const std::string& name)
{
  std::map<std::string,image>::const_iterator it = Images.find(name);
  if (it == Images.end()) {
    METLIBS_LOG_ERROR("image not found: '" << name << "'");
    return 0;
  }
  const image& im = it->second;

  readImage(name);
  if (im.type == marker) {
    double max=0.0, min=0.0; // always include point (0,0) in calculation
    for (const Line& il : im.line) {
      for (const QPointF& p : il.points) {
        if (p.x() > max)
          max = p.x();
        if (p.x() < min)
          min = p.x();
      }
    }
    return int(max - min);
  } else {
    return im.width;
  }
}

int ImageGallery::heightp(const std::string& name)
{
  std::map<std::string,image>::const_iterator it = Images.find(name);
  if (it == Images.end()) {
    METLIBS_LOG_ERROR("image not found: '" << name << "'");
    return 0;
  }
  const image& im = it->second;

  readImage(name);
  if (im.type == marker) {
    double max=0.0, min=0.0; // always include point (0,0) in calculation
    for (const Line& il : im.line) {
      for (const QPointF& p : il.points) {
        if (p.y() > max)
          max = p.y();
        if (p.y() < min)
          min = p.y();
      }
    }
    return int(max - min);
  } else {
    return im.height;
  }
}

bool ImageGallery::delImage(const std::string& name)
{
  if (!Images.count(name)){
    METLIBS_LOG_ERROR("ImageGallery::delImage ERROR image not found:"
    << name);
    return false;
  }
  Images[name].erase();
  return true;
}

bool ImageGallery::delPattern(const std::string& name)
{
  if (!Patterns.count(name)){
    METLIBS_LOG_ERROR("ImageGallery::delPattern ERROR pattern not found:"
    << name);
    return false;
  }
  Patterns[name].erase();
  return true;
}


bool ImageGallery::plotImage_(DiGLPainter* gl, StaticPlot* sp,
    const std::string& name, float gx, float gy,
    float scalex, float scaley, int alpha)
{
  if (gx < sp->getPlotSize().x1 || gx >= sp->getPlotSize().x2 ||
      gy < sp->getPlotSize().y1 || gy >= sp->getPlotSize().y2)
    return true;

  std::map<std::string,image>::const_iterator it = Images.find(name);
  if (it == Images.end()) {
    METLIBS_LOG_ERROR("image not found: '" << name << "'");
    return 0;
  }
  const image& im = it->second;

  int nx= im.width;
  int ny= im.height;
  DiGLPainter::GLenum glformat= DiGLPainter::gl_RGBA;
  int ncomp= 4;

  if (!im.alpha){
    glformat= DiGLPainter::gl_RGB;
    ncomp= 3;
  }

  gl->RasterPos2f(gx,gy);

  if (alpha != 255 && ncomp == 4 /*&& !sp->hardcopy*/) {
    int fsize = nx*ny*ncomp;
    unsigned char* newdata= new unsigned char [fsize];
    unsigned char av= static_cast<unsigned char>(alpha);
    for (int j=0; j<fsize; j++){
      newdata[j]= im.data[j];
      // if original alpha-value lower: keep it
      if ((j+1) % 4 == 0 && newdata[j] > av)
        newdata[j] = av;
    }

    gl->DrawPixels((DiGLPainter::GLint)nx, (DiGLPainter::GLint)ny,
        glformat, DiGLPainter::gl_UNSIGNED_BYTE, newdata);

    delete[] newdata;

  } else {
    gl->DrawPixels((DiGLPainter::GLint)nx, (DiGLPainter::GLint)ny,
        glformat, DiGLPainter::gl_UNSIGNED_BYTE, im.data);
  }

  return true;
}

bool ImageGallery::plotMarker_(DiGLPainter* gl, StaticPlot* sp,
    const std::string& name, float x, float y, float scale)
{
  METLIBS_LOG_SCOPE(LOGVAL(name));
  if (x < sp->getPlotSize().x1 || x >= sp->getPlotSize().x2 ||
      y < sp->getPlotSize().y1 || y >= sp->getPlotSize().y2)
    return true;

  std::map<std::string,image>::const_iterator it = Images.find(name);
  if (it == Images.end()) {
    METLIBS_LOG_ERROR("image not found: '" << name << "'");
    return false;
  }
  const image& im = it->second;

  if (!im.line.empty()) {
    diutil::GlMatrixPushPop pushpop(gl);
    gl->Translatef(x,y,0.0);
    float Scalex= scale*sp->getPhysToMapScaleX()*0.7;
    float Scaley= Scalex;
    gl->Scalef(Scalex,Scaley,0.0);

    for (const Line& il : im.line) {
      METLIBS_LOG_DEBUG(LOGVAL(il.width) << LOGVAL(il.colour) << LOGVAL(il.circle) << LOGVAL(il.fill)
                        << LOGVAL(il.radius) << LOGVAL(il.points.size()));
      if (il.width > 0)
        gl->LineWidth(il.width);
      if (!il.colour.empty())
        gl->setColour(Colour(il.colour));
      if (il.circle) {
        if (il.radius > 0)
          gl->drawCircle(il.fill, 0, 0, il.radius);
      } else {
        if (il.fill)
          gl->drawPolygon(il.points);
        else
          gl->drawPolyline(il.points);
      }
    }
  }
  return true;
}

bool ImageGallery::readFile(const std::string& name, const std::string& filename)
{
  METLIBS_LOG_SCOPE(LOGVAL(name) << LOGVAL(filename));
  ifstream inFile;
  std::string line;
  vector<std::string> vline;

  inFile.open(filename.c_str(),ios::in);
  if (inFile.bad()) {
    METLIBS_LOG_ERROR("ImageGallery: Can't open file: " << filename);
    return false;
  }

  while (getline(inFile,line)) {
    if (line.length()>0) {
      miutil::trim(line);
      if (line.length()>0 && line[0]!='#')
        vline.push_back(line);
    }
  }

  inFile.close();

  std::vector<Line>& lines = Images[name].line;
  Line l;
  for (const std::string& vl : vline) {
    METLIBS_LOG_DEBUG(LOGVAL(vl));
    const vector<std::string> tokens = miutil::split(vl, " ");
    if (tokens.size() != 2)
      continue;
    const std::string& t0 = tokens[0];
    if (t0 == "mvto" || l.circle) {
      METLIBS_LOG_DEBUG(LOGVAL(l.width) << LOGVAL(l.colour) << LOGVAL(l.circle) << LOGVAL(l.fill)
                        << LOGVAL(l.radius) << LOGVAL(l.points.size()));
      if (l.valid())
        lines.push_back(l);
      l.invalidate();
    }
    if (t0 == "lto" || t0 == "mvto") {
      vector<std::string> coor = miutil::split(tokens[1], ",");
      if(coor.size() != 2)
        continue;
      l.points << QPointF(atof(coor[0].c_str()), atof(coor[1].c_str()));
    } else if (t0 == "circle") {
      const int r = atoi(tokens[1].c_str());
      if (r > 0) {
        l.circle = true;
        l.radius = r;
        METLIBS_LOG_DEBUG(LOGVAL(l.radius));
      }
    } else if (t0 == "lw") {
      l.width = atoi(tokens[1].c_str());
    } else if (t0 == "colour") {
      l.colour = tokens[1];
    } else if (t0 == "mode") {
      l.fill = (tokens[1] == "fill");
    }
  }
  if (l.valid()) {
    METLIBS_LOG_DEBUG(LOGVAL(l.width) << LOGVAL(l.colour) << LOGVAL(l.circle) << LOGVAL(l.fill)
                      << LOGVAL(l.radius) << LOGVAL(l.points.size()));
    lines.push_back(l);
  }

  return !lines.empty();
}

bool ImageGallery::plotImage(DiGLPainter* gl, StaticPlot* sp, const std::string& name,
    float x, float y, bool center, float scale, int alpha)
{
  METLIBS_LOG_SCOPE();
  if(!readImage(name))
    return false;

  if (!Images.count(name)){
    METLIBS_LOG_ERROR("image not found: '" << name << "'");
    return false;
  }

  if(Images[name].type == marker)
    return plotMarker_(gl, sp, name, x, y, scale);

  if (Images[name].data==0) {
    METLIBS_LOG_ERROR("ImageGallery::plot ERROR no image-data:"
    << name);
    return false;
  }

  float gx= x, gy= y; // raster position
  int nx= Images[name].width;
  int ny= Images[name].height;
  float scalex=scale, scaley=scale;

  if (center){
    // center image on x,y: find scale
    float sx= sp->getPhysToMapScaleX() * 0.5;
    float sy= sp->getPhysToMapScaleY() * 0.5;
    gx-= nx*sx*scale;
    gy-= ny*sy*scale;
  }

  gl->Enable(DiGLPainter::gl_BLEND);
  gl->BlendFunc(DiGLPainter::gl_SRC_ALPHA, DiGLPainter::gl_ONE_MINUS_SRC_ALPHA);

  gl->PixelZoom(scalex,scaley);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_ROWS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_PIXELS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ROW_LENGTH,nx);

  bool res= plotImage_(gl, sp, name, gx, gy, scalex, scaley, alpha);

  //Reset gl
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_ROWS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_PIXELS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ROW_LENGTH,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ALIGNMENT,4);
  gl->Disable(DiGLPainter::gl_BLEND);

  return res;
}


bool ImageGallery::plotImages(DiGLPainter* gl, StaticPlot* sp, int n,
    const vector<std::string>& vn,
    const float* x, const float* y,
    bool center, float scale, int alpha)
{
  if (n == 0){
    METLIBS_LOG_ERROR("ImageGallery::plotImages ERROR no positions:");
    return false;
  }
  if (n != int(vn.size())){
    METLIBS_LOG_ERROR("ImageGallery::plotImages ERROR names and positions do not match:"
    << n);
    return false;
  }

  int nx = 0;
  int ny = 0;
  float scalex=scale, scaley=scale;
  std::string oldname;
  float sx= scale*0.5*sp->getPhysToMapScaleX();
  float sy= scale*0.5*sp->getPhysToMapScaleY();

  gl->Enable(DiGLPainter::gl_BLEND);
  gl->BlendFunc(DiGLPainter::gl_SRC_ALPHA, DiGLPainter::gl_ONE_MINUS_SRC_ALPHA);

  gl->PixelZoom(scalex,scaley);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_ROWS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_PIXELS,0);

  for (int j=0; j<n; j++){
    if (!Images.count(vn[j])){
      METLIBS_LOG_ERROR("ImageGallery::plotImages ERROR image not found:"
      << vn[j]);
      return false;
    }

    if(!readImage(vn[j])) return false;

    if(Images[vn[j]].type == marker) {
      plotMarker_(gl, sp, vn[j], x[j], y[j], scale);
      continue;
    }

    if (Images[vn[j]].data==0) {
      METLIBS_LOG_ERROR("ImageGallery::plotImages ERROR no image-data:"
      << vn[j]);
      return false;
    }

    float gx= x[j], gy= y[j]; // raster position
    if (j == 0 || vn[j] != oldname){
      nx= Images[vn[j]].width;
      ny= Images[vn[j]].height;
      gl->PixelStorei(DiGLPainter::gl_UNPACK_ROW_LENGTH,nx);
    }

    if (center){
      // center image on x,y
      gx-= nx*sx;
      gy-= ny*sy;
    }

    plotImage_(gl, sp, vn[j], gx, gy, scalex, scaley, alpha);
    oldname = vn[j];
  }

  //Reset gl
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_ROWS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_PIXELS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ROW_LENGTH,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ALIGNMENT,4);
  gl->Disable(DiGLPainter::gl_BLEND);

  return true;
}

bool ImageGallery::plotImages(DiGLPainter* gl, StaticPlot* sp, const int n,
    const std::string& name,
    const float* x, const float* y,
    bool center, float scale, int alpha)
{
  if(!readImage(name))
    return false;

  if (n == 0){
    METLIBS_LOG_ERROR("ImageGallery::plotImages ERROR no positions:");
    return false;
  }

  vector<std::string> vn(n,name);
  return plotImages(gl, sp, n, vn, x, y, center, scale, alpha);
}


bool ImageGallery::plotImageAtPixel(DiGLPainter* gl, StaticPlot* sp,
    const std::string& name, float x, float y,
    bool center, float scale, int alpha)
{
  if(!readImage(name))
    return false;

  if (!Images.count(name)){
    METLIBS_LOG_ERROR("ImageGallery::plot ERROR image not found:" << name);
    return false;
  }

  if(Images[name].type == marker)
    return plotMarker_(gl, sp, name, x, y, scale);

  if (Images[name].data==0) {
    METLIBS_LOG_ERROR("ImageGallery::plot ERROR no image-data:"
    << name);
    return false;
  }

  float gx= x, gy= y; // raster position
  int nx= Images[name].width;
  int ny= Images[name].height;
  float scalex=scale, scaley=scale;

  if (center){
    // center image on x,y
    gx-=  nx*scale/2.0;
    gy-=  ny*scale/2.0;
  }

  sp->PhysToMap(XY(gx, gy)).unpack(gx, gy);

  gl->Enable(DiGLPainter::gl_BLEND);
  gl->BlendFunc(DiGLPainter::gl_SRC_ALPHA, DiGLPainter::gl_ONE_MINUS_SRC_ALPHA);

  gl->PixelZoom(scalex,scaley);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_ROWS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_PIXELS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ROW_LENGTH,nx);

  bool res= plotImage_(gl, sp, name, gx, gy, scalex, scaley, alpha);

  //Reset gl
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_ROWS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_SKIP_PIXELS,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ROW_LENGTH,0);
  gl->PixelStorei(DiGLPainter::gl_UNPACK_ALIGNMENT,4);
  gl->Disable(DiGLPainter::gl_BLEND);

  return res;
}

DiGLPainter::GLubyte* ImageGallery::getPattern(std::string name)
{

  if(!readPattern(name)) return 0;
  return Patterns[name].pattern_data;

}

void ImageGallery::printInfo() const
{
  map<std::string,image>::const_iterator p= Images.begin();
  for( ; p!=Images.end(); p++){
    METLIBS_LOG_INFO("Image: " << p->second.name
    << " W:" << p->second.width
    << " H:" << p->second.height
    << " A:" << (p->second.alpha ? "YES" : "NO"));
  }
}

void ImageGallery::ImageNames(vector<std::string>& vnames,
    int type) const
{
  vnames.clear();

  //   map<std::string,image>::const_iterator p= Images.begin();

  //   for (; p!=Images.end(); p++)
  //     vnames.push_back(p->first);
  int n=Type[type].size();
  for(int i=0; i<n; i++)
    vnames.push_back(Type[type][i]);
}

std::string ImageGallery::getFilename(const std::string& name, bool pattern)
{
  if(pattern)
    return Patterns[name].filename;

  //images, markers ...
  return Images[name].filename;
}

bool ImageGallery::parseSetup()
{
  //  METLIBS_LOG_DEBUG("ImageGallery: parseSetup");
  const std::string ig_name = "IMAGE_GALLERY";
  vector<std::string> sect_ig;

  if (!SetupParser::getSection(ig_name,sect_ig)){
    METLIBS_LOG_ERROR(ig_name << " section not found");
    return false;
  }

  for(unsigned int i=0; i<sect_ig.size(); i++) {
    vector<std::string> token = miutil::split(sect_ig[i], "=");

    if(token.size() != 2){
      std::string errmsg="Line must contain '='";
      SetupParser::errorMsg(ig_name,i,errmsg);
      return false;
    }

    std::string key = miutil::to_lower(token[0]);
    std::string value = token[1];
    //      METLIBS_LOG_DEBUG("key: "<<key);
    //      METLIBS_LOG_DEBUG("Value: "<<value);
    if(miutil::contains(key, "path")){
      miutil::replace(key, "path","");
      value += "/*";
    }
    int type;
    if     (key=="basic")   type = basic;
    else if(key=="pattern") type = fillpattern;
    else if(key=="marker")  type = marker;
    else continue;

    const diutil::string_v matches = diutil::glob(value);
    for (diutil::string_v::const_iterator it = matches.begin(); it != matches.end(); ++it) {
      const std::string& fname = *it;
      if((miutil::contains(fname, ".png") || miutil::contains(fname, ".xpm"))
          && not miutil::contains(fname, "~"))
        addImageName(fname,type);
    }
  }
  return true;
}
