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

#ifndef EDITITEMSLAYERGROUPSPANE_H
#define EDITITEMSLAYERGROUPSPANE_H

#include <QWidget>
#include <QSharedPointer>

class QVBoxLayout;
class QToolButton;

namespace EditItems {

class LayerGroup;
class LayerManager;
class ScrollArea;
class ClickableLabel;

class LayerGroupWidget : public QWidget
{
  Q_OBJECT
public:
  LayerGroupWidget(const QSharedPointer<LayerGroup> &, bool, QWidget * = 0);
  ~LayerGroupWidget();
  const QSharedPointer<LayerGroup> layerGroup() const;
  void updateLabels();
  void showInfo(bool);
private:
  QSharedPointer<LayerGroup> layerGroup_;
  ClickableLabel *nameLabel_;
  ClickableLabel *infoLabel_;
signals:
  void mouseClicked(QMouseEvent *);
};

class LayerGroupsPane : public QWidget
{
  Q_OBJECT
public:
  LayerGroupsPane(LayerManager *layerManager);
  void showInfo(bool);
private:
  QVBoxLayout *layout_;
  ScrollArea *scrollArea_; // ### necessary to keep as a member? (cf. activeLayersScrollArea_)
  QToolButton *addToDefaultLGFromFileButton_;
  QToolButton *addToNewLGFromFileButton_;
  bool showInfo_;
  void addWidgetForLG(const QSharedPointer<LayerGroup> &);
  QList<LayerGroupWidget *> allWidgets();
  void removeWidget(LayerGroupWidget *);
  void addToLGFromFile(bool);
  void updateWidgetStructure();
  LayerManager *layerManager;

private slots:
  void addToDefaultLGFromFile();
  void addToNewLGFromFile();
  void mouseClicked(QMouseEvent *);
  void updateWidgetContents();
signals:
  void updated();
};

} // namespace

#endif // EDITITEMSLAYERGROUPSPANE_H