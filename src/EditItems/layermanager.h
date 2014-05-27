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

#ifndef EDITITEMSLAYERMANAGER_H
#define EDITITEMSLAYERMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QList>
#include <EditItems/drawingitembase.h>

#define CurrLayer layerManager->currentLayer()
#define CurrEditLayer layerManager->currentLayer(true)

namespace EditItems {

class LayerGroup;
class Layer;

class LayerManager : public QObject
{
  Q_OBJECT
public:
  LayerManager();
  bool isEmpty() const;

  void resetFirstDefaultLayer();
  QSharedPointer<Layer> currentLayer(bool = false) const;
  void setCurrentLayer(const QSharedPointer<Layer> &);
  QSharedPointer<LayerGroup> &defaultLayerGroup();
  void addToNewLayerGroup(const QList<QSharedPointer<Layer> > &, const QString & = QString());
  void addToDefaultLayerGroup(const QList<QSharedPointer<Layer> > &);
  void addToDefaultLayerGroup(const QSharedPointer<Layer> &);
  void replaceInDefaultLayerGroup(const QList<QSharedPointer<Layer> > &);
  QSharedPointer<LayerGroup> createNewLayerGroup(const QString &) const;
  QSharedPointer<Layer> createNewLayer(const QString & = QString()) const;
  QSharedPointer<Layer> createDuplicateLayer(const QSharedPointer<Layer> &) const;
  void mergeLayers(const QList<QSharedPointer<Layer> > &, const QSharedPointer<Layer> &) const;
  const QList<QSharedPointer<LayerGroup> > &layerGroups() const;
  const QList<QSharedPointer<Layer> > &orderedLayers() const;
  QSharedPointer<LayerGroup> findLayerGroup(const QString &) const;
  QSharedPointer<LayerGroup> findLayerGroup(const QSharedPointer<Layer> &) const;
  QSharedPointer<Layer> findLayer(const QString &) const;
  void removeLayer(const QSharedPointer<Layer> &);
  void moveLayer(const QSharedPointer<Layer> &, const QSharedPointer<Layer> &);

private:
  QList<QSharedPointer<LayerGroup> > layerGroups_;
  // the layers of all layer groups organized in a single, ordered list:
  QList<QSharedPointer<Layer> > orderedLayers_;
  QSharedPointer<Layer> currLayer_;

  void addToLayerGroup(QSharedPointer<LayerGroup> &, const QList<QSharedPointer<Layer> > &);

  QString createUniqueLayerGroupName(const QString &) const;
  void ensureUniqueLayerGroupName(const QSharedPointer<LayerGroup> &) const;

  QString createUniqueLayerName(const QString &) const;
  void ensureUniqueLayerName(const QSharedPointer<Layer> &) const;
};

} // namespace

#endif // EDITITEMSLAYERMANAGER_H