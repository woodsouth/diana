/*
  Diana - A Free Meteorological Visualisation Tool

  $Id$

  Copyright (C) 2013 met.no

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
#ifndef _diDrawingManager_h
#define _diDrawingManager_h

#include "GL/gl.h"

#include <EditItems/drawingitembase.h>

#include <diCommonTypes.h>
#include <diDrawingTypes.h>
#include "diManager.h"
#include <diMapMode.h>

#include <diField/diGridConverter.h>

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QSet>
#include <QString>
#include <QVariantMap>

#include <vector>

class DrawingItemBase;
class PlotModule;
class ObjectManager;

class QAction;
class QKeyEvent;
class QMouseEvent;

#if defined(USE_PAINTGL)
#include "PaintGL/paintgl.h"
#define QGLContext PaintGLContext
#endif

/**
  \brief Manager for drawing areas and annotations.
*/
class DrawingManager : public Manager
{
  Q_OBJECT

public:
  class itemCompare
  {
  public:
      inline bool operator()(const DrawingItemBase* t1, const DrawingItemBase* t2) const
      {
          return (t1->id() < t2->id());
      }
  };

  DrawingManager();
  ~DrawingManager();

  /// parse DRAWING section of setup file (defines Drawing products)
  bool parseSetup();

  virtual bool changeProjection(const Area& newArea);
  virtual bool loadItems(const QString &fileName);
  virtual bool prepare(const miutil::miTime &time);
  virtual void plot(bool under, bool over);
  virtual bool processInput(const std::vector<std::string>& inp);

  virtual void sendMouseEvent(QMouseEvent* event, EventResult& res) {}
  virtual void sendKeyboardEvent(QKeyEvent* event, EventResult& res) {}

  QList<QPointF> getLatLonPoints(DrawingItemBase* item) const;
  void setFromLatLonPoints(DrawingItemBase* item, const QList<QPointF> &latLonPoints);
  QList<QPointF> PhysToGeo(const QList<QPointF> &points) const;
  QList<QPointF> GeoToPhys(const QList<QPointF> &latLonPoints);

  QSet<DrawingItemBase *> getItems() const;

  virtual DrawingItemBase *createItemFromVarMap(const QVariantMap &vmap, QString *error);

  template<typename BaseType, typename PolyLineType, typename SymbolType>
  inline BaseType *createItemFromVarMap_(const QVariantMap &vmap, QString *error)
  {
    Q_ASSERT(!vmap.empty());
    Q_ASSERT(vmap.contains("type"));
    Q_ASSERT(vmap.value("type").canConvert(QVariant::String));
    BaseType *item = 0;
    *error = QString();
    if (vmap.value("type").toString().endsWith("PolyLine")) {
      item = new PolyLineType();
    } else if (vmap.value("type").toString().endsWith("Symbol")) {
      item = new SymbolType();
    } else {
      *error = QString("unsupported item type: %1, expected %2 or %3")
          .arg(vmap.value("type").toString()).arg("*PolyLine").arg("*Symbol");
    }
    if (item) {
      Drawing(item)->setProperties(vmap);
      setFromLatLonPoints(Drawing(item), Drawing(item)->getLatLonPoints());
    }

    QString typeName = vmap.value("Style:Type").toString();
    PolygonStyle *style = 0;

    if (typeName.isEmpty())
      typeName = "Default";

    // If the style is not a custom style, try to find it and use it as the item's style.

    if (typeName != "Custom") {
      if (polygonStyles.contains(typeName))
        style = &polygonStyles[typeName];
    }

    // Create a style for each item if the style is a custom style, or if the named type was not found.

    if (!style) {
      PolygonStyle *style = new PolygonStyle();
      QHash<QString, QString> definition;
      foreach (QString key, vmap.keys()) {
        if (key.startsWith("Style:"))
          definition[key.mid(6)] = vmap[key].toString();
      }
      style->parse(definition);
    }
    item->setStyle(style);

    return item;
  }

  static DrawingManager *instance();

  // Resource handling
  void drawSymbol(const QString &name, float x, float y, int width, int height);

  PolygonStyle *getPolygonStyle(const QString &name = QString());

  // Dialog-related methods
  QSet<QString> &drawings();

public slots:
  std::vector<miutil::miTime> getTimes() const;

protected:
  virtual void addItem_(DrawingItemBase *);
  virtual void removeItem_(DrawingItemBase *item);

  Rectangle plotRect;
  Rectangle editRect;
  Area currentArea;

  QSet<DrawingItemBase *> items_;

private:
  std::string timeProperty(const QVariantMap &properties, std::string &time_str) const;

  GridConverter gc;
  QSet<QString> drawings_;

  QHash<QString, QByteArray> symbols;
  QHash<QString, GLuint> symbolTextures;
  QHash<QString, QImage> imageCache;
  QHash<QString, PolygonStyle> polygonStyles;

  static DrawingManager *self;  // singleton instance pointer
};

#endif
