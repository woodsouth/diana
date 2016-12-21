/*
 Diana - A Free Meteorological Visualisation Tool

 Copyright (C) 2006-2016 met.no

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

#include "MovieMaker.h"
#include "util/subprocess.h"
#include <QImage>

#include <cstdlib>

#define MILOGGER_CATEGORY "diana.MovieMaker"
#include <miLogger/miLogging.h>

using namespace std;

MovieMaker::MovieMaker(const QString &filename, const QString &format,
    float delay, const QSize& frameSize)
  : mOutputFile(filename)
  , mOutputFormat(format)
  , mDelay(delay)
  , mFrameSize(frameSize)
  , mFrameCount(0)
{
  if (!isImageFormat()) {
    char tmpl[256];
    snprintf(tmpl, sizeof(tmpl), "%s/diana-video-XXXXXX", QDir::tempPath().toStdString().c_str());
    const char* outdir = mkdtemp(tmpl);
    if (outdir)
      mOutputDir = QDir(outdir);
    else
      METLIBS_LOG_ERROR("could not create temp dir for movie generation");
  } else {
    mOutputDir = QDir(filename);
  }
}

MovieMaker::~MovieMaker()
{
  if (!isImageFormat() && mOutputDir.exists()) {
    QString outdir = mOutputDir.absolutePath();
    METLIBS_LOG_DEBUG("cleaning up '" << outdir.toStdString() << "' ...");
    for (int i=1; i<=mFrameCount; ++i) {
      if (!mOutputDir.remove(frameFile(i)))
        METLIBS_LOG_ERROR("could not remove '"<< frameFile(i).toStdString() << "' in '" << outdir.toStdString() << "'");
    }
    if (diutil::execute("rmdir", QStringList() << outdir) != 0)
      METLIBS_LOG_ERROR("could not remove directory '" << outdir.toStdString() << "'");
  }
}

bool MovieMaker::isImageFormat() const
{
  return mOutputFormat == "img";
}

static QString framePattern("frame_%1.png");
static int framePatternWidth = 3;
static QString framePatternFF = framePattern.arg("%0" + QString::number(framePatternWidth) + "d");

static QString converter = "avconv";

QString MovieMaker::frameFile(int frameNumber) const
{
  return framePattern.arg(frameNumber, framePatternWidth, 10, QLatin1Char('0'));
}

QString MovieMaker::framePath(int frameNumber) const
{
  return mOutputDir.filePath(frameFile(frameNumber));
}

bool MovieMaker::addImage(const QImage &image)
{
  if (!mOutputDir.exists())
    return false;

  const QImage::Format FORMAT = QImage::Format_RGB32;

  QImage imageScaled;
  if (image.size() == mFrameSize)
    imageScaled = image;
  else
    imageScaled = image.scaled(mFrameSize);

  if (imageScaled.format() != FORMAT)
    imageScaled = imageScaled.convertToFormat(FORMAT);

  mFrameCount += 1;
  return imageScaled.save(framePath(mFrameCount));
}

bool MovieMaker::finish()
{
  if (mFrameCount == 0) {
    METLIBS_LOG_WARN("no video frames in '" << mOutputFile.toStdString() << "'");
    return false;
  }
  if (isImageFormat())
    return true;
  if (!mOutputDir.exists())
    return false;

  const int framerate_n = 10, framerate_d = std::max(1, (int)(mDelay*framerate_n + 0.5f));

  QString encoder;
  if (mOutputFormat == "mp4")
    encoder = "libx264";
  else if (mOutputFormat == "avi")
    encoder = "msmpeg4v2";
  else
    encoder = "mpeg2video";

  QStringList args;
  args << "-y" // overwrite output file
       << "-framerate" << QString("%1/%2").arg(framerate_n).arg(framerate_d)
       << "-i" << mOutputDir.filePath(framePatternFF)
       << "-c:v" << encoder
       << "-r" << "30"
       << "-pix_fmt" << "yuv420p"
       << mOutputFile;
  METLIBS_LOG_INFO("running: '" << converter.toStdString() << "' '" << args.join("' '").toStdString() << "'");
  const int code = diutil::execute(converter, args);
  if (code == 0) {
    METLIBS_LOG_INFO("video created in '" << args.back().toStdString() << "'");
  } else {
    METLIBS_LOG_ERROR("problem creating video, code=" << code);
  }
  return (code == 0);
}