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
#include <qapplication.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qlistbox.h>
#include <qtToggleButton.h>
#include <qtUtility.h>
#include <diVprofManager.h>
#include <qtVprofModelDialog.h>

#define HEIGHTLISTBOX 100

/***************************************************************************/

VprofModelDialog::VprofModelDialog( QWidget* parent,VprofManager * vm )
  : QDialog(parent),vprofm(vm)
{
#ifdef DEBUGPRINT 
  cout<<"VprofModelDialog::VprofModelDialog called"<<endl;
#endif

  //caption to appear on top of dialog
  setCaption(tr("Diana Vertical Profiles - Models"));
  
  // string constants
  ASFIELD     =  tr("As field").latin1();
  OBSTEMP     =  tr("Observations:TEMP").latin1();
  OBSPILOT    =  tr("Observations:PILOT").latin1();
  OBSAMDAR    =  tr("Observations:AMDAR").latin1();

  // send translated menunames to manager
  map<miString,miString> textconst;
  textconst["ASFIELD"]  = ASFIELD;
  textconst["OBSTEMP"]  = OBSTEMP;
  textconst["OBSPILOT"] = OBSPILOT;
  textconst["OBSAMDAR"] = OBSAMDAR;
  vm->setMenuConst(textconst);
  
//********** create the various QT widgets to appear in dialog ***********

  //**** the three buttons "auto", "tid", "fil" *************
  vector<miString> model;
  model.push_back(tr("Model").latin1());
  model.push_back(tr("File").latin1());

  modelfileBut = new QButtonGroup( this );
  modelButton = new ToggleButton(modelfileBut, tr("Model").latin1());
  fileButton = new ToggleButton(modelfileBut, tr("File").latin1());
  QHBoxLayout* modelfileLayout = new QHBoxLayout(modelfileBut);
  modelfileLayout->addWidget(modelButton);
  modelfileLayout->addWidget(fileButton);
  modelfileBut->setExclusive( true );
  modelfileBut->setButton(0);
  //modelfileClicked is called when model,file buttons clicked 
  connect( modelfileBut, SIGNAL( clicked(int) ), 
	   SLOT( modelfileClicked(int) ) );

  //if a model is selected- should be as model- else default model(hirlam)

  //********** the list of files/models to choose from **************

  modelfileList = new QListBox( this);
  modelfileList->setMinimumHeight(HEIGHTLISTBOX);
  modelfileList->setSelectionMode( QListBox::Multi );
  modelfileList->setEnabled(true);  
  m_modelfileButIndex = 0;
  updateModelfileList();

  //push button to show help
  QPushButton * modelhelp = NormalPushButton( tr("Help"), this );
  connect(  modelhelp, SIGNAL(clicked()), SLOT( helpClicked()));

  //push button to delete
  QPushButton * deleteAll = NormalPushButton( tr("Delete all"), this );
  connect( deleteAll, SIGNAL(clicked()),SLOT(deleteAllClicked()));

  //push button to refresh
  QPushButton * refresh = NormalPushButton( tr("Refresh"), this );
  connect(  refresh, SIGNAL(clicked()), SLOT( refreshClicked()));

  //push button to hide dialog
  QPushButton * modelhide = NormalPushButton( tr("Hide"), this );
  connect( modelhide, SIGNAL(clicked()), SIGNAL(ModelHide()));

   //push button to apply the selected command and then hide dialog
  QPushButton  * modelapplyhide = NormalPushButton( tr("Apply+Hide"), this );
  connect( modelapplyhide, SIGNAL(clicked()), SLOT(applyhideClicked()));

  //push button to apply the selected command
  QPushButton * modelapply = NormalPushButton( tr("Apply"), this );
  connect( modelapply, SIGNAL(clicked()), SLOT(applyClicked()));

  // ************ place all the widgets in layouts ****************

  //place buttons "oppdater", "hjelp" etc. in horizontal layout
  QHBoxLayout* hlayout1 = new QHBoxLayout( 5 );
  hlayout1->addWidget( modelhelp );
  hlayout1->addWidget( deleteAll );
  hlayout1->addWidget( refresh );

  //place buttons "utf�r", "help" etc. in horizontal layout
  QHBoxLayout* hlayout2 = new QHBoxLayout( 5 );
  hlayout2->addWidget( modelhide );
  hlayout2->addWidget( modelapplyhide );
  hlayout2->addWidget( modelapply );


  //create a vertical layout to put all widgets and layouts in
  QVBoxLayout * vlayout = new QVBoxLayout( this, 5, 5 );                     
  vlayout->addWidget( modelfileBut );
  vlayout->addWidget( modelfileList );
  vlayout->addLayout( hlayout1 );
  vlayout->addLayout( hlayout2 );
}

/*********************************************/

void VprofModelDialog::modelfileClicked(int tt){
  //this slot is called when modelfile button pressed
#ifdef DEBUGPRINT
  cerr <<"VprofModelDialog::modelfileClicked()\n";
#endif

  //update the index to current item in time file button
  m_modelfileButIndex = tt;

  updateModelfileList();
}
  

/*********************************************/

void VprofModelDialog::refreshClicked(){
  //this slot is called when refresh button pressed
#ifdef DEBUGPRINT
  cerr <<"VprofModelDialog::refreshClicked()\n";
#endif
  updateModelfileList();

}

/*********************************************/

void VprofModelDialog::deleteAllClicked(){
  //this slot is called when delete button pressed
#ifdef DEBUGPRINT
  cerr <<"VprofModelDialog::deleteAllClicked()\n";
#endif
  modelfileList->clearSelection();
}

/*********************************************/

void VprofModelDialog::helpClicked(){
  //this slot is called when help button pressed
#ifdef DEBUGPRINT
  cerr <<"VprofModelDialog::helpClicked()\n";
#endif
  emit showdoc("ug_verticalprofiles.html");
}


/*********************************************/

void VprofModelDialog::applyClicked(){
  //this slot is called when apply button pressed
#ifdef DEBUGPRINT
  cerr <<"VprofModelDialog::applyClicked(int tt)\n";
#endif
  setModel();
  emit ModelApply();

}

/*********************************************/

void VprofModelDialog::applyhideClicked(){
  //this slot is called when applyhide button pressed
#ifdef DEBUGPRINT
  cerr <<"VprofModelDialog::applyhideClicked(int tt)\n";
#endif
  setModel();
  emit ModelHide();
  emit ModelApply();

}


/*********************************************/
void VprofModelDialog::setSelection(){
#ifdef DEBUGPRINT
  cerr<< "VprofModelDialog::setSelection()" << endl;
#endif
  if (m_modelfileButIndex==0){
    vector <miString> models = vprofm->getSelectedModels();
    int n = models.size();
    for (int i = 0;i<n;i++){
      miString model = models[i];
      int m = modelfileList->count();
      for (int j = 0;j<m;j++){
	miString listModel =  modelfileList->text(j).latin1();
	if (model==listModel) modelfileList->setSelected(j,true);
      }
    }
  }
}

/*********************************************/
void VprofModelDialog::setModel(){
#ifdef DEBUGPRINT
  cerr<< "VprofModelDialog::setModel()" << endl;
#endif

  bool showObsTemp =false;
  bool showObsPilot=false;
  bool showObsAmdar=false;
  bool asField=false;

  if (m_modelfileButIndex==0){

    vector <miString> models;
    int n = modelfileList->count();
    for (int i = 0; i<n;i++){
      if(modelfileList->isSelected(i)){
	miString model = modelfileList->text(i).latin1();
	if(model==OBSTEMP)
	  showObsTemp=true;
	else if(model==OBSPILOT)
	  showObsPilot=true;
	else if(model==OBSAMDAR)
	  showObsAmdar=true;
	else if (model==ASFIELD)
	  asField = true;
	else
	  models.push_back(model);
      }
    }
    vprofm->setSelectedModels(models,asField,
			      showObsTemp,showObsPilot,showObsAmdar);

  } else if (m_modelfileButIndex==1){

    vector <miString> files;
    int n = modelfileList->count();
    for (int i = 0; i<n;i++){
      if(modelfileList->isSelected(i)){
	miString file = modelfileList->text(i).latin1(); 
	files.push_back(file);
      }
    }
    vprofm->setSelectedFiles(files,asField,
			     showObsTemp,showObsPilot,showObsAmdar);
  }
  
}
/*********************************************/

void VprofModelDialog::updateModelfileList(){
#ifdef DEBUGPRINT
  cerr << "VprofModelDialog::updateModelfileList()\n";
#endif

  //want to keep th selected models/files
  int n= modelfileList->count();
  set<miString> current;
  for (int i=0; i<n; i++)
    if (modelfileList->isSelected(i))
      current.insert(miString(modelfileList->text(i).latin1()));

  //clear box with list of files 
  modelfileList->clear();

  if (m_modelfileButIndex==0){
    //make a string list with models to insert into modelfileList
    vector <miString> modelnames =vprofm->getModelNames();
    int nr_models = modelnames.size();
    modelfileList->insertItem(OBSTEMP);
    modelfileList->insertItem(OBSPILOT);
    modelfileList->insertItem(OBSAMDAR);
    modelfileList->insertItem(ASFIELD);
    for (int i = 0;i<nr_models;i++){
      modelfileList->insertItem(modelnames[i].c_str());
    }

    //insert into modelfilelist
  } else if (m_modelfileButIndex==1){
    //make a string list with files to insert into modelfileList
    vector <miString> modelfiles =vprofm->getModelFiles();
    int nr_files = modelfiles.size();
    for (int i = 0;i<nr_files;i++){
      modelfileList->insertItem(modelfiles[i].c_str());
    }
  }

  set<miString>::iterator pend= current.end();
  n= modelfileList->count();
  for (int i=0; i<n; i++)
    if (current.find(miString(modelfileList->text(i).latin1()))!=pend)
      modelfileList->setSelected(i,true);
  
}


bool VprofModelDialog::close(bool alsoDelete){
#ifdef DEBUGPRINT
  cerr <<"VprofModel was closed!" << endl;
#endif
  emit ModelHide();
  return true;
}
