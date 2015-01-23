/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2015 met.no

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
#include "vcross_v2/VcrossQuickmenues.h"
#include "VcrossQtWidget.h"

#include "diLogFile.h"
#include "diPrintOptions.h"

#include <QDialog>
#include <QVariant>

#include <vector>

namespace miutil {
class miTime;
}
class ActionButton;
class Controller;
struct LocationData;
class ToggleButton;
class VcrossStyleDialog;
class VcrossSetupDialog;
class Ui_VcrossWindow;

class QAction;
class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QPrinter;
class QString;
class QSpinBox;

#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE /* empty */
#endif

/**
  \brief Window for Vertical Crossections

  Contains a crossection window, toolbars and menues.
  Receives and sends "events" from/to DianaMainWindow.
*/
class VcrossWindow: public QWidget
{
  Q_OBJECT

public:
  VcrossWindow();
  ~VcrossWindow();

  // ========== begin called via VcrossWindowInterface
  void makeVisible(bool visible);
  bool changeCrossection(const std::string& crossection);
  void mainWindowTimeChanged(const miutil::miTime& t);
  void parseQuickMenuStrings(const std::vector<std::string>& vstr);
  void parseSetup();

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
  void requestLoadCrossectionFiles(const QStringList& filenames);
  //! called when draw/edit button is toggled
  void requestVcrossEditor(bool on);
  void crossectionChanged(const QString&);
  void crossectionSetChanged(const LocationData& locations);
  void emitTimes(const std::string&, const std::vector<miutil::miTime>&);
  void setTime(const std::string&, const miutil::miTime&);
  void quickMenuStrings(const std::string&, const std::vector<std::string>&);
  void nextHVcrossPlot();
  void prevHVcrossPlot();

protected:
  void closeEvent(QCloseEvent*);

private:
  //! setup UI and adjust some things that cannot be done easily with qt designer
  void setupUi();

  // emits SIGNAL(crossectionSetChanged) with list of crossections from vcross-manager

  void enableDynamicCsIfSupported();
  void emitCrossectionSet();

  void repaintPlotIfNotInGroup();
  void repaintPlot();

private Q_SLOTS:
  // GUI slots for leayer buttons
  void onFieldAction(int position, int action);

  // slots for QtManager
  void onFieldChangeBegin(bool fromScript);
  void onFieldAdded(const std::string& model, const std::string& field, int position);
  void onFieldRemoved(const std::string& model, const std::string& field, int position);
  void onFieldOptionsChanged(const std::string& model, const std::string& field, int position);
  void onFieldVisibilityChanged(const std::string& model, const std::string& field, int position);
  void onFieldChangeEnd();

  void crossectionChangedSlot(int);
  void crossectionListChangedSlot();
  void timeChangedSlot(int);
  void timeListChangedSlot();

  // GUI slots for widget
  void stepCrossection(int direction);
  void stepTime(int direction);

  // GUI slots for window
  void onAddField();
  void onRemoveAllFields();
  void leftCrossectionClicked();
  void rightCrossectionClicked();
  void crossectionBoxActivated(int index);
  void leftTimeClicked();
  void rightTimeClicked();
  void timeBoxActivated(int index);

  void printClicked();
  void saveClicked();
  void onShowSetupDialog();
  void timeGraphClicked(bool on);
  void quitClicked();
  void helpClicked();

  // ---------- slots for vcross setup dialog
  //! called when the apply from setup dialog is clicked
  void changeSetup();

private:
  vcross::QtManager_p vcrossm;
  std::auto_ptr<vcross::VcrossQuickmenues> quickmenues;

  std::auto_ptr<Ui_VcrossWindow> ui;

  VcrossStyleDialog* vcStyleDialog;
  VcrossSetupDialog* vcSetupDialog;
  bool dynEditManagerConnected;

  // printerdefinitions
  printOptions priop;
  QString mRasterFilename;

  int vcrossDialogX, vcrossDialogY;

  bool firstTime, active;
  bool mInFieldChangeGroup;
  bool mGroupChangedFields;
};

#endif // _qt_vcrossmainwindow_
