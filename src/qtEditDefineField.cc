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
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QToolTip>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <qtUtility.h>
#include <qtEditDefineField.h>
#include <diController.h>
#include <diEditManager.h>

#include <iostream>


/*********************************************/
EditDefineFieldDialog::EditDefineFieldDialog(QWidget* parent,
					     Controller* llctrl, 
					     int n,EditProduct ep)
  : QDialog(parent,"definefield", true), m_ctrl(llctrl), num(n), EdProd(ep)
{

  m_editm= m_ctrl->getEditManager();  

  if (num==-1){
    fieldname= tr("Objects").latin1();
    setCaption(tr("Pick objects for editing"));
  }
  else if (num < EdProd.fields.size()){
    fieldname=EdProd.fields[num].name;
    setCaption(tr("Pick fields for editing"));
  }

  MODELFIELDS = tr("Model fields").latin1();

  miString txt= fieldname + " " + miString(tr("from:").latin1());
  QLabel* mainlabel= TitleLabel( txt.cStr(), this );
  
  productNames=getProductNames();
  prodnamebox = ComboBox( this, productNames, true, 0);
  connect( prodnamebox, SIGNAL( activated(int) ),
	   SLOT( prodnameActivated(int) )  );


  fBox= new QListWidget(this);

  connect(fBox, SIGNAL(itemClicked(QListWidgetItem *)), 
	  SLOT(fieldselect(QListWidgetItem *)));


  QString xps= tr("Official product") + " -- <i>" +
    tr("Locally stored") ;
  QLabel* xplabel= new QLabel(xps, this);

  QVBoxLayout* vlayout = new QVBoxLayout( this);

  vlayout->addWidget( mainlabel );
  vlayout->addWidget( prodnamebox );
  vlayout->addWidget( fBox );
  vlayout->addSpacing(10);


  vlayout->addSpacing(10);
  vlayout->addWidget( xplabel );
  vlayout->addSpacing(10);

  QFrame *line;
  // Create a horizontal frame line
  line = new QFrame( this );
  line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  vlayout->addWidget( line );


  //*** the box (with label) showing which files/fields have been choosen ****
  if (num==-1)
      filesLabel = TitleLabel(tr("Selected objects"), this);
  else
    filesLabel = TitleLabel(tr("Selected fields"), this);
  filenames = new QListWidget( this );
  connect(filenames, SIGNAL(itemClicked( QListWidgetItem *)), 
			    SLOT(filenameSlot(QListWidgetItem *)));

  vlayout->addWidget( filesLabel );
  vlayout->addWidget( filenames );


  if (num==-1){
    //*****  Checkboxes for selecting fronts/symbols/areas  **********

    cbs0= new QCheckBox(tr("Fronts"), this);
    cbs1= new QCheckBox(tr("Symbols"),this);
    cbs2= new QCheckBox(tr("Areas"),  this);
    cbs3= new QCheckBox(tr("Form"),   this);

    connect(cbs0, SIGNAL(clicked()), SLOT(cbsClicked()));
    connect(cbs1, SIGNAL(clicked()), SLOT(cbsClicked()));
    connect(cbs2, SIGNAL(clicked()), SLOT(cbsClicked()));
    connect(cbs3, SIGNAL(clicked()), SLOT(cbsClicked()));
    initCbs();

    vlayout->addWidget(cbs0);
    vlayout->addWidget(cbs1);
    vlayout->addWidget(cbs2);
    vlayout->addWidget(cbs3);
  }


  //push buttons to delete all selections
  Delete = NormalPushButton( tr("Delete"), this );
  connect( Delete, SIGNAL(clicked()), SLOT(DeleteClicked()));

  //push button to refresh filelists
  refresh =NormalPushButton( tr("Refresh"), this );
  connect( refresh, SIGNAL( clicked() ), SLOT( Refresh() )); 

  //place  "delete" and "refresh" buttons in hor.layout
  QHBoxLayout* h0layout = new QHBoxLayout();
  h0layout->addWidget( Delete );
  h0layout->addWidget( refresh );

  QHBoxLayout* hlayout = new QHBoxLayout();

  ok= NormalPushButton( tr("OK"), this);
  QPushButton* cancel= NormalPushButton( tr("Cancel"), this);
  hlayout->addWidget( ok );
  hlayout->addWidget( cancel );
  connect( ok, SIGNAL(clicked()),  SLOT(accept()) );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
  ok->setEnabled(false);

  vlayout->addLayout( h0layout );
  vlayout->addLayout( hlayout );

  //check existing selections for product
  if (num==-1 && EdProd.objectprods.size())
    vselectedprod=EdProd.objectprods;
  else if (num>-1 && num < EdProd.fields.size()){
    if (EdProd.fields[num].fromfield)
      selectedfield=EdProd.fields[num].fromfname;  
    else
      vselectedprod.push_back(EdProd.fields[num].fromprod);
  }

  selectedProdIndex=-1;

  //update filenames with existing selections
  updateFilenames();

  if (num>-1 && fields.size()==0){
    if (prodnamebox->count()>1) prodnamebox->setCurrentItem(1);
    prodnameActivated(1); 
  }
  else
    prodnameActivated(0);

  
}//end constructor EditDefineFieldDialog


/*********************************************/

vector <miString> EditDefineFieldDialog::getProductNames(){
#ifdef dEditDlg 
  cout << "getProductNames called " << endl;
#endif
  vector <miString> name;
  if (!m_editm) return name;
  //get fields
  if (num>-1){
    name.push_back(MODELFIELDS);
    fields= m_editm->getValidEditFields(EdProd,num);   
  }
  name.push_back(EdProd.name);
  vector<savedProduct> sp=
    m_editm->getSavedProducts(EdProd,num);
  pmap[EdProd.name]=sp;
  int n = EdProd.inputproducts.size();
  for (int i =0;i<n;i++){
    if (EdProd.inputproducts[i]==EdProd.name) continue;  
    EditProduct epin;
    if (m_editm->findProduct(epin,EdProd.inputproducts[i])){
      vector<savedProduct> spin;
      if (num>-1)
	spin = m_editm->getSavedProducts(epin,EdProd.fields[num].name);
      else
	spin = m_editm->getSavedProducts(epin,num);
      if (spin.size()){
	name.push_back(epin.name);
	pmap[epin.name]=spin;
      }
    }
  }
  return name;
}

/*********************************************/

void EditDefineFieldDialog::prodnameActivated(int iprod){
#ifdef dEditDlg 
  cout << "EditDefineFieldDialog::prodnameActivated " << iprod << endl;
#endif
  if (productNames.size()>iprod){
    currentProductName= productNames[iprod];
    fillList();
  }
}

/*********************************************/

void EditDefineFieldDialog::fillList()
{
#ifdef dEditDlg 
  cout << "EditDefineFieldDialog::fillList for " << currentProductName << endl;
#endif
  if (currentProductName.empty()) return;
  fBox->clear();
  fBox->clearFocus();
  if (currentProductName==MODELFIELDS){
    for (int i=0; i<fields.size(); i++){
      fBox->addItem(QString(fields[i].cStr()));
    }
  } else {
    vector <savedProduct> splist = pmap[currentProductName];
    for (int i=0; i<splist.size(); i++){
      miString str = splist[i].pid + miString(" - ") +splist[i].ptime.isoTime();
      QListWidgetItem* item = new QListWidgetItem(QString(str.cStr()));
      bool italic= (splist[i].source==data_local);
      QFont font = item->font();
      font.setItalic(italic);
      item->setFont(font);
      fBox->addItem(item);
    }
  }
}

/*********************************************/

void EditDefineFieldDialog::fieldselect(QListWidgetItem* item)
{
#ifdef dEditDlg 
  cout << "EditDefineFieldDialog::fieldselect" << endl;
#endif
  int i =fBox->row(item);
  if (currentProductName==MODELFIELDS){
    selectedfield= fields[i];
    //clear selected products, not possible to have both fields and products
    vselectedprod.clear();
    updateFilenames();
    if (filenames->count()) filenames->item(0)->setSelected(true);
  } else {
    vector <savedProduct> splist = pmap[currentProductName];
    savedProduct selectedprod = splist[i];
    //check this savedproduct not already selected
    vector<savedProduct>::iterator p=vselectedprod.begin();
    for (; p!=vselectedprod.end() && selectedprod.filename!=p->filename;p++);
    if (p==vselectedprod.end()){
      initCbs();
      selectedprod.selectObjectTypes=selectedObjectTypes();
      if (num>-1) vselectedprod.clear(); //only one field !
      vselectedprod.push_back(selectedprod);
      selectedProdIndex=vselectedprod.size()-1;
    }
    //clear selected field, not possible to have both fields and products
    selectedfield.clear();
    updateFilenames();
    if (selectedProdIndex>-1 && selectedProdIndex < filenames->count())
      filenames->item(selectedProdIndex)->setSelected(true);
  }
  ok->setEnabled(true);

}

/*********************************************/

void EditDefineFieldDialog::updateFilenames(){
#ifdef dEditDlg 
  cout << "EditDefineFieldDialog::updateFilenames" << endl;
#endif
  filenames->clear();
  if (fieldSelected()){
    miString namestr=selectedfield;
    if (!namestr.empty()) {
      filenames->addItem(QString(namestr.c_str()));
    }
  }
  if (productSelected()){
    for (int i = 0;i<vselectedprod.size();i++){
      miString namestr=m_editm->savedProductString(vselectedprod[i]);
      if (!namestr.empty()) {
	filenames->addItem(QString(namestr.c_str()));
      }      
    }
  } 
}

/*********************************************/

void EditDefineFieldDialog::filenameSlot(QListWidgetItem* item){
#ifdef dEditDlg 
  cout << "EditDefineFieldDialog::filenameSlot" << endl;
#endif
  selectedProdIndex=filenames->row(item);
  if (num==-1){
    map<miString,bool> useEditobject = 
      m_ctrl->decodeTypeString(vselectedprod[selectedProdIndex].selectObjectTypes);
    setCheckedCbs(useEditobject);
  }
}

/*********************************************/
void EditDefineFieldDialog::DeleteClicked(){
#ifdef dEditDlg 
  cout<<"EditDefineFieldDialog::DeleteClicked() called;" << endl;
#endif
  if (fieldSelected()){
    selectedfield.clear();
    updateFilenames();
  }
  else if (productSelected()&&selectedProdIndex<vselectedprod.size()){
    vselectedprod.erase(vselectedprod.begin()+selectedProdIndex);
    selectedProdIndex--;
    if (selectedProdIndex<0 && vselectedprod.size()) selectedProdIndex=0;
    updateFilenames();
    if (selectedProdIndex>-1 && selectedProdIndex < filenames->count())
      filenames->item(selectedProdIndex)->setSelected(true);
    else
      initCbs();
  }
  //refresh List, clear selection
  fillList();
  if (vselectedprod.size() && selectedProdIndex > -1) ok->setEnabled(true);
}

/*********************************************/

void EditDefineFieldDialog::Refresh(){
#ifdef dEditDlg 
  cout<<"EditDefineFieldDialog::Refresh() called;" << endl;
#endif
  getProductNames();
  fillList();
}
/***********************************************************/

void EditDefineFieldDialog::cbsClicked(){
#ifdef dEditDlg 
  cout << "cbs0Clicked !" << endl;
#endif
  if (num>-1) return;
  if (productSelected()){
    if (selectedProdIndex < vselectedprod.size()){
      vselectedprod[selectedProdIndex].selectObjectTypes=selectedObjectTypes();
      updateFilenames();
      if (selectedProdIndex>-1 && selectedProdIndex < filenames->count())
	filenames->item(selectedProdIndex)->setSelected(true);
    }
  }
  if (vselectedprod.size() && selectedProdIndex > -1) ok->setEnabled(true);
}



void EditDefineFieldDialog::setCheckedCbs(map<miString,bool> useEditobject){
  if (num>-1) return;
  if (useEditobject["front"])
    cbs0->setChecked(true);
  else
    cbs0->setChecked(false);
  if (useEditobject["symbol"])
    cbs1->setChecked(true);
  else
    cbs1->setChecked(false);
  if (useEditobject["area"])
    cbs2->setChecked(true);
  else
    cbs2->setChecked(false);
  if (useEditobject["anno"])
    cbs3->setChecked(true);
  else
    cbs3->setChecked(false);
}


void EditDefineFieldDialog::initCbs(){
  if (num>-1) return;
  cbs0->setChecked(true);
  cbs1->setChecked(true);
  cbs2->setChecked(true);
  cbs3->setChecked(true);
}


miString EditDefineFieldDialog::selectedObjectTypes() {
  //fronts /symbols/areas ?
  if (num>-1) return miString(" ");
  miString str;
  str+="types=";
  
  if (cbs0->isChecked()) str+="front,";
  if (cbs1->isChecked()) str+="symbol,";
  if (cbs2->isChecked()) str+="area,";
  if (cbs3->isChecked()) str+="anno";

  return str;
}


/***********************************************************/








