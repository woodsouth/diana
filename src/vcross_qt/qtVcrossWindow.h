/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2017 met.no

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

#ifndef _qt_vcrossmainwindow_
#define _qt_vcrossmainwindow_

#include "vcross_v2/VcrossQtManager.h"
#include "VcrossQtWidget.h"

#include "diLogFile.h"
#include "diPrintOptions.h"

#include <QVariant>
#include <QWidget>

#include <vector>

namespace miutil {
class miTime;
}
class ActionButton;
class Controller;
struct LocationData;
class ToggleButton;
class VcrossAddPlotDialog;
class VcrossImageSource;
class VcrossReplaceModelDialog;
class VcrossStyleDialog;
class VcrossSetupDialog;
class Ui_VcrossWindow;

class QAction;
class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QString;
class QSpinBox;
class QStatusBar;

/**
  \brief Window for Vertical Crossections

  Contains a crossection window, toolbars and menues.
  Receives and sends "events" from/to DianaMainWindow.
*/
class VcrossWindow: public QWidget
{
  Q_OBJECT

public:
  VcrossWindow(vcross::QtManager_p vcm);
  ~VcrossWindow();

  // ========== begin called via VcrossWindowInterface
  void makeVisible(bool visible);

  void writeLog(LogFileIO& logfile);
  void readLog(const LogFileIO& logfile, const std::string& thisVersion, const std::string& logVersion,
      int displayWidth, int displayHeight);

  void dynCrossEditManagerChange(const QVariantMap &props);
  void dynCrossEditManagerRemoval(int id);
  void slotCheckEditmode(bool editing);
  // ========== end called via VcrossWindowInterface

Q_SIGNALS: // defined in VcrossInterface
  void VcrossHide();
  void requestHelpPage(const std::string&, const std::string& = ""); // activate help
  //! called when draw/edit button is toggled
  void requestVcrossEditor(bool on, bool tg);
  void vcrossHistoryNext();
  void vcrossHistoryPrevious();

protected:
  void closeEvent(QCloseEvent*);

private:
  //! setup UI and adjust some things that cannot be done easily with qt designer
  void setupUi();

  void connectRestartAddAfterReplace();

  // emits SIGNAL(crossectionSetChanged) with list of crossections from vcross-manager

  void enableDynamicCsIfSupported();
  void enableTimeGraphIfSupported();

  void repaintPlotIfNotInGroup();
  void repaintPlot();

  void ensureStyleDialog();

  VcrossImageSource* imageSource();

private Q_SLOTS:
  // from layer button bar
  void onRequestStyleEditor(int position);

  // slots for QtManager
  void onFieldChangeBegin(bool fromScript);
  void onFieldAdded(int position);
  void onFieldRemoved(int position);
  void onFieldOptionsChanged(int position);
  void onFieldVisibilityChanged(int position);
  void onFieldChangeEnd();

  void crossectionChangedSlot(int);
  void crossectionListChangedSlot();
  void timeChangedSlot(int);
  void timeListChangedSlot();

  void timeGraphModeChangedSlot(bool);

  // GUI slots for widget
  void stepCrossection(int direction);
  void stepTime(int direction);

  // GUI slots for window
  void onAddField();
  void onShowStyleDialog();
  void onRemoveAllFields();
  void onReplaceModel();
  void leftCrossectionClicked();
  void rightCrossectionClicked();
  void crossectionBoxActivated(int index);
  void leftTimeClicked();
  void rightTimeClicked();
  void timeBoxActivated(int index);
  void onRealizationChanged(int value);

  void printClicked();
  void saveClicked();
  void onShowSetupDialog();
  void onToggleDrawEdit(bool enableEdit);
  void timeGraphClicked(bool on);
  void quitClicked();
  void helpClicked();

  void onAxisPosition(const QString& text);

  // ---------- slots for vcross setup dialog
  //! called when the apply from setup dialog is clicked
  void changeSetup();

private:
  std::unique_ptr<Ui_VcrossWindow> ui;
  QStatusBar* mStatusBar;

  vcross::QtManager_p vcrossm;

  VcrossAddPlotDialog* vcAddPlotDialog;
  VcrossReplaceModelDialog* vcReplaceModelDialog;
  VcrossStyleDialog* vcStyleDialog;
  VcrossSetupDialog* vcSetupDialog;
  // positions of dialogs from the logfile
  QPoint mPositionAddPlot;
  QPoint mPositionReplaceModel;
  QPoint mPositionStyle;
  QPoint mPositionSetup;

  bool dynEditManagerConnected;

  // printerdefinitions
  printOptions priop;
  std::unique_ptr<VcrossImageSource> imageSource_;

  bool firstTime, active;
  bool mInFieldChangeGroup;
  bool mGroupChangedFields;
};

#endif // _qt_vcrossmainwindow_
