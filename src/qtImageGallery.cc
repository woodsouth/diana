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
#include <qtImageGallery.h>
#include <diImageGallery.h>
#include <diSetupParser.h>
#include <glob.h>

map<miString,QImage> QtImageGallery::Images;

QtImageGallery::QtImageGallery()
{
}

bool QtImageGallery::addImageToGallery(const miString name,
				       const QImage& image)
{
  if (image.isNull()){
    cerr << "qtImageGallery::addImageToGallery ERROR:"
	 << " invalid image:" << name << endl;
    return false;
  }
  ImageGallery ig;
  unsigned char* data= 0;
  int width=  image.width();
  int height= image.height();
  int numb= width*height*4;
  int k=0;
  bool addok= false;
  if (numb==0){
    cerr << "qtImageGallery::addImageToGallery ERROR:"
	 << " zero bytes in image:" << name << endl;
    return false;
  } else {
    data= new unsigned char [numb];
    for (int j=height-1; j>=0; j--){
      for (int i=0; i<width; i++){
	QRgb p= image.pixel(i,j);
	data[k+0]= qRed(p);
	data[k+1]= qGreen(p);
	data[k+2]= qBlue(p);
	data[k+3]= qAlpha(p);
	k+= 4;
      }
    }

    addok= ig.addImage(name,width,height,
		       data,true);
    delete[] data;
  }

  if (!addok) return false;

  // add image to local list
  Images[name] = image;
  
  return true;
}

bool QtImageGallery::addImageToGallery(const miString name,
				       miString& imageStr)
{
  vector<miString> vs= imageStr.split(" ");
  int n=vs.size();
  QByteArray a(n);
  for (int i=0; i<n; i++)
    a[i]= char(atoi(vs[i].cStr()));

  QDataStream s( a, IO_ReadOnly );  // open on a's data
  QImage image;
  s >> image;                       // read raw bindata

  return addImageToGallery(name, image);

}

void QtImageGallery::clear()
{
  ImageGallery ig;
  ig.clear();
  
  Images.clear();
}


bool QtImageGallery::delImage(const miString& name)
{
  ImageGallery ig;
  
  if ((Images.count(name) == 0) || (!ig.delImage(name)))
    return false;
  
  Images.erase(name);

  return true;
}

void QtImageGallery::ImageNames(vector<miString>& vnames) const
{
  vnames.clear();
  
  map<miString,QImage>::const_iterator p= Images.begin();

  for (; p!=Images.end(); p++)
    vnames.push_back(p->first);
}

bool QtImageGallery::Image(const miString& name,
			   QImage& image) // return QImage
{
  if (Images.count(name) == 0)
    return false;

  image= Images[name];
  return true;
}
