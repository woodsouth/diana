/*
 Diana - A Free Meteorological Visualisation Tool

 Copyright (C) 2006-2018 met.no

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

#include "diFieldPlotManager.h"

#include "diana_config.h"

#include "diField/diFieldManager.h"
#include "diFieldPlot.h"
#include "diFieldPlotCommand.h"
#include "diFieldUtil.h"
#include "util/misc_util.h"
#include "util/string_util.h"

#include "diField/diFlightLevel.h"

#include <puTools/miStringFunctions.h>

#include <memory>

#define MILOGGER_CATEGORY "diana.FieldPlotManager"
#include <miLogger/miLogging.h>

using namespace std;
using namespace miutil;

struct PlotFieldInput
{
  std::string name;
  bool is_standard_name;
};

struct FieldPlotManagerPlotField
{
  std::string name;                  ///< the field name in dialog etc.
  std::string fieldgroup;            ///< special fieldgroup name (to separate some fields)
  std::vector<PlotFieldInput> input; ///< the input fields, read or computed
  std::string inputstr;              // same as above, used as tooltip
  std::set<std::string> vcoord;
  FieldFunctions::VerticalType vctype;
};

FieldPlotManager::FieldPlotManager()
    : fieldManager(new FieldManager)
{
}

FieldPlotManager::~FieldPlotManager()
{
}

FieldPlot* FieldPlotManager::createPlot(const PlotCommand_cp& pc)
{
  FieldPlotCommand_cp cmd = std::dynamic_pointer_cast<const FieldPlotCommand>(pc);
  if (!cmd)
    return 0;
  std::unique_ptr<FieldPlot> fp(new FieldPlot(this));
  if (fp->prepare(cmd->field.plot, cmd))
    return fp.release();
  else
    return 0;
}

bool FieldPlotManager::parseSetup()
{
  fieldManager->parseSetup(); // FIXME this ignores errors

  if (!parseFieldPlotSetup())
    return false;
  if (!parseFieldGroupSetup())
    return false;
  fieldManager->setFieldNames(getFields());
  return true;
}

bool FieldPlotManager::parseFieldPlotSetup()
{
  METLIBS_LOG_SCOPE();

  std::string sect_name = "FIELD_PLOT";
  vector<std::string> lines;

  if (!SetupParser::getSection(sect_name, lines)) {
    METLIBS_LOG_ERROR(sect_name << " section not found");
    return true;
  }

  vPlotField.clear();
  const std::string key_loop = "loop";
  const std::string key_field = "field";
  const std::string key_endfield = "end.field";
  const std::string key_fieldgroup = "fieldgroup";
  const std::string key_plot = "plot";
  const std::string key_plottype = "plottype";
  const std::string key_vcoord = "vcoord";
  const std::string key_vc_type = "vc_type";

  // parse setup

  int nlines = lines.size();

  vector<std::string> vstr;
  std::string key, str;
  vector<std::string> vpar;

  vector<std::string> loopname;
  vector<vector<std::string> > loopvars;
  map<std::string,int>::const_iterator pfp;
  int firstLine = 0,lastLine,nv;
  bool waiting= true;

  for (int l = 0; l < nlines; l++) {

    vstr = splitComStr(lines[l], true);
    int n = vstr.size();
    key = miutil::to_lower(vstr[0]);

    if (waiting) {
      if (key == key_loop && n >= 4) {
        vpar.clear();
        for (unsigned int i = 3; i < vstr.size(); i++) {
          vpar.push_back(diutil::quote_removed(vstr[i]));
        }
        loopname.push_back(vstr[1]);
        loopvars.push_back(vpar);
      } else if (key == key_field) {
        firstLine = l;
        waiting = false;
      }
    } else if (key == key_endfield) {
      lastLine = l;

      unsigned int nl = loopname.size();
      if (nl > loopvars.size()) {
        nl = loopvars.size();
      }
      unsigned int ml = 1;
      if (nl > 0) {
        ml = loopvars[0].size();
        for (unsigned int il = 0; il < nl; il++) {
          if (ml > loopvars[il].size()) {
            ml = loopvars[il].size();
          }
        }
      }

      for (unsigned int m = 0; m < ml; m++) {
        std::string name;
        std::string fieldgroup;
        vector<std::string> input;
        std::string inputstr;
        set<std::string> vcoord;
        FieldFunctions::VerticalType vctype = FieldFunctions::vctype_none;

        for (int i = firstLine; i < lastLine; i++) {
          str = lines[i];
          for (unsigned int il = 0; il < nl; il++) {
            miutil::replace(str, loopname[il], loopvars[il][m]);
          }
          if (i == firstLine) {
            // resplit to keep names with ()
            vstr = miutil::split(str, "=", false);
            if (vstr.size() < 2) {
              std::string errm = "Missing field name";
              SetupParser::errorMsg(sect_name, i, errm);
              continue;
            }
            name = diutil::quote_removed(vstr[1]);
          } else {
            vstr = splitComStr(str, false);
            nv = vstr.size();
            int j = 0;
            while (j < nv - 2) {
              key = miutil::to_lower(vstr[j]);
              if (key == key_plot && vstr[j + 1] == "=" && j < nv - 3) {
                const miutil::KeyValue_v option1(1, miutil::KeyValue(key_plottype, vstr[j + 2]));
                if (!updateFieldPlotOptions(name, option1)) {
                  std::string errm = "|Unknown fieldplottype in plotcommand";
                  SetupParser::errorMsg(sect_name, i, errm);
                  break;
                }
                inputstr = vstr[j + 3].substr(1, vstr[j + 3].length() - 2);
                input = miutil::split(inputstr, ",", true);
                if (input.size() < 1 || input.size() > 5) {
                  std::string errm = "Bad specification of plot arguments";
                  SetupParser::errorMsg(sect_name, i, errm);
                  break;
                }

                const miutil::KeyValue_v option2(1, miutil::KeyValue("dim", miutil::from_number(int(input.size()))));
                if (!updateFieldPlotOptions(name, option2)) {
                  std::string errm = "|Unknown fieldplottype in plotcommand";
                  SetupParser::errorMsg(sect_name, i, errm);
                  break;
                }
              } else if (key == key_fieldgroup && vstr[j + 1] == "=") {
                fieldgroup = diutil::quote_removed(vstr[j + 2]);
              } else if (key == key_vcoord && vstr[j + 1] == "=") {
                vector<std::string> vcoordTokens = miutil::split(vstr[j+2], ",");
                for( size_t ii=0; ii<vcoordTokens.size(); ++ii ) {
                  vcoord.insert(vcoordTokens[ii]);
                }
              } else if (key == key_vc_type && vstr[j + 1] == "=") {
                vctype = FieldFunctions::getVerticalType(vstr[j+2]);
              } else if (vstr[j + 1] == "=") {
                // this should be a plot option
                const miutil::KeyValue_v option1(1, miutil::KeyValue(vstr[j], vstr[j + 2]));
                if (!updateFieldPlotOptions(name, option1)) {
                  SetupParser::errorMsg(sect_name, i, "Something wrong in plotoption specifications");
                  break;
                }
              } else {
                std::string errm = "Unknown keyword in field specifications: " + vstr[0];
                SetupParser::errorMsg(sect_name, i, errm);
                break;
                //j-=2;
              }
              j += 3;
            }
          }
        }

        if (!name.empty() && !input.empty()) {
          PlotField_p pf = std::make_shared<FieldPlotManagerPlotField>();
          pf->name = name;
          pf->fieldgroup = fieldgroup;
          pf->input.reserve(input.size());
          for (const auto& i : input) {
            const std::vector<std::string> ii = miutil::split(i, ":");
            PlotFieldInput pfi;
            pfi.name = ii[0];
            pfi.is_standard_name = (ii.size() == 2 && ii[1] == "standard_name");
            pf->input.push_back(pfi);
          }
          pf->inputstr = inputstr;
          pf->vcoord = vcoord;
          pf->vctype = vctype;
          vPlotField.push_back(pf);
        }
      }

      loopname.clear();
      loopvars.clear();
      waiting = true;
    }
  }

  return true;
}

bool FieldPlotManager::parseFieldGroupSetup()
{
  std::string sect_name = "FIELD_GROUPS";
  vector<std::string> lines;

  if (!SetupParser::getSection(sect_name, lines)) {
    METLIBS_LOG_ERROR(sect_name << " section not found");
    return true;
  }

  const std::string key_name = "name";
  const std::string key_group = "group";

  int nlines = lines.size();

  for (int l = 0; l < nlines; l++) {
    vector<std::string> tokens= miutil::split_protected(lines[l], '"','"');
    if ( tokens.size()== 2 ) {
      vector<std::string> stokens= miutil::split_protected(tokens[0], '"','"',"=",true);
      if (stokens.size() == 2 && stokens[0] == key_name ){
        std::string name = stokens[1];
        stokens= miutil::split_protected(tokens[1], '"','"',"=",true);
        if (stokens.size() == 2 && stokens[0] == key_group ){
          groupNames[stokens[1]] = name;
        }
      }
    }
  }

  return true;
}

bool FieldPlotManager::updateFieldFileSetup(const std::vector<std::string>& lines, std::vector<std::string>& errors)
{
  return fieldManager->updateFileSetup(lines, errors);
}

// static
vector<std::string> FieldPlotManager::splitComStr(const std::string& s, bool splitall)
{
  // split commandstring into tokens.
  // split on '=', ',' and multiple blanks, keep blocks within () and ""
  // split on ',' only if <splitall> is true

  vector<std::string> tmp;

  int i = 0, j = 0, n = s.size();
  if (n) {
    while (i < n && s[i] == ' ') {
      i++;
    }
    j = i;
    for (; i < n; i++) {
      if (s[i] == '=') { // split on '=', but keep it
        if (i - j > 0) {
          tmp.push_back(s.substr(j, i - j));
        }
        tmp.push_back("=");
        j = i + 1;
      } else if (s[i] == ',' && splitall) { // split on ','
        if (i - j > 0) {
          tmp.push_back(s.substr(j, i - j));
        }
        j = i + 1;
      } else if (s[i] == '(') { // keep () blocks
        if (i - j > 0) {
          tmp.push_back(s.substr(j, i - j));
        }
        j = i;
        i++;
        while (i < n && s[i] != ')') {
          i++;
        }
        tmp.push_back(s.substr(j, (i < n) ? (i - j + 1) : (i - j)));
        j = i + 1;
      } else if (s[i] == '"') { // keep "" blocks
        if (i - j > 0) {
          tmp.push_back(s.substr(j, i - j));
        }
        j = i;
        i++;
        while (i < n && s[i] != '"') {
          i++;
        }
        tmp.push_back(s.substr(j, (i < n) ? (i - j + 1) : (i - j)));
        j = i + 1;
      } else if (s[i] == ' ') { // split on (multiple) blanks
        if (i - j > 0) {
          tmp.push_back(s.substr(j, i - j));
        }
        while (i < n && s[i] == ' ') {
          i++;
        }
        j = i;
        i--;
      } else if (i == n - 1) {
        tmp.push_back(s.substr(j, i - j + 1));
      }
    }
  }

  return tmp;
}

vector<std::string> FieldPlotManager::getFields()
{
  set<std::string> paramSet;
  for (const auto& pf : vPlotField) {
    for (const auto& input : pf->input)
      paramSet.insert(input.name);
  }

  return vector<std::string>(paramSet.begin(), paramSet.end());
}

plottimes_t FieldPlotManager::getFieldTime(const std::vector<FieldPlotCommand_cp>& pinfos, bool updateSources)
{
  METLIBS_LOG_SCOPE();

  std::vector<FieldRequest> requests;
  for (FieldPlotCommand_cp cmd : pinfos) {
    std::vector<FieldRequest> fieldrequest;
    std::string plotName;
    parsePin(cmd, cmd->field, fieldrequest, plotName);
    diutil::insert_all(requests, fieldrequest);
  }

  if (requests.empty())
    return plottimes_t();

  return getFieldTime(requests, updateSources);
}

miTime FieldPlotManager::getFieldReferenceTime(FieldPlotCommand_cp cmd)
{
  METLIBS_LOG_SCOPE();

  std::string plotName;
  FieldRequest frq;
  vector<std::string> paramNames;
  parseString(cmd, cmd->field, frq, paramNames, plotName);

  const std::string timestr = fieldManager->getBestReferenceTime(frq.modelName, frq.refoffset, frq.refhour);
  if (timestr.empty())
    return miTime();
  return miTime(timestr);
}

void FieldPlotManager::getCapabilitiesTime(plottimes_t& normalTimes, int& timediff, const PlotCommand_cp& pc)
{
  METLIBS_LOG_SCOPE(LOGVAL(pc->toString()));
  //Finding times from pinfo
  //TODO: find const time

  FieldPlotCommand_cp cmd = std::dynamic_pointer_cast<const FieldPlotCommand>(pc);
  if (!cmd)
    return;

  //finding timediff
  timediff = 0;
  for (const KeyValue& kv : cmd->options()) {
    if (kv.key() == "ignore_times" && kv.toBool() == true) {
      normalTimes.clear();
      return;
    }
    if (kv.key() == "timediff") {
      timediff = kv.toInt();
    }
  }

  //getting times
  normalTimes = getFieldTime(std::vector<FieldPlotCommand_cp>(1, cmd), true);

  METLIBS_LOG_DEBUG("no. of times"<<normalTimes.size());
}

vector<std::string> FieldPlotManager::getFieldLevels(FieldPlotCommand_cp cmd)
{
  std::vector<std::string> levels;

  std::vector<FieldRequest> vfieldrequest;
  std::string plotName;
  parsePin(cmd, cmd->field, vfieldrequest, plotName);

  if (!vfieldrequest.size())
    return levels;

  std::map<std::string, FieldPlotInfo> fieldPlotInfo;
  fieldManager->getFieldPlotInfo(vfieldrequest[0].modelName, vfieldrequest[0].refTime, fieldPlotInfo);
  std::map<std::string, FieldPlotInfo>::const_iterator ip = fieldPlotInfo.find(vfieldrequest[0].paramName);

  if (ip != fieldPlotInfo.end()) {
    levels = ip->second.vlevels();
  }

  return levels;
}

plottimes_t FieldPlotManager::getFieldTime(std::vector<FieldRequest>& request, bool updateSources)
{
  METLIBS_LOG_SCOPE();

  for (FieldRequest& frq : request) {
    if (frq.predefinedPlot) {
      std::vector<FieldRequest> fr = getParamNames(frq.paramName, frq);
      if (!fr.empty()) {
        frq.paramName = fr[0].paramName;
        frq.standard_name = fr[0].standard_name;
      }
    }
    flightlevel2pressure(frq);
  }

  return fieldManager->getFieldTime(request, updateSources);
}

bool FieldPlotManager::addGridCollection(const std::string& modelname, const std::string& filename, bool writeable)
{
  const std::string gridiotype("fimex");

  std::vector<std::string> config;
  const std::vector<std::string> filenames(1, filename);
  const std::vector<std::string> format(1, "netcdf");
  std::vector<std::string> option;
  if (writeable)
    option.push_back("writeable=true");
  return fieldManager->addGridCollection(gridiotype, modelname, filenames, format, config, option);
}

FieldModelGroupInfo_v FieldPlotManager::getFieldModelGroups()
{
  return fieldManager->getFieldModelGroups();
}

set<std::string> FieldPlotManager::getFieldReferenceTimes(const std::string& model)
{
  return fieldManager->getReferenceTimes(model);
}

std::map<std::string, std::string> FieldPlotManager::getFieldGlobalAttributes(const std::string& modelName, const std::string& refTime)
{
  return fieldManager->getGlobalAttributes(modelName, refTime);
}

bool FieldPlotManager::makeFields(FieldPlotCommand_cp cmd, const miTime& const_ptime, vector<Field*>& vfout)
{
  METLIBS_LOG_SCOPE();
  if (cmd->hasMinusField())
    return makeDifferenceField(cmd, const_ptime, vfout);
  else
    return makeFields(cmd, cmd->field, const_ptime, vfout);
}

bool FieldPlotManager::makeFields(FieldPlotCommand_cp cmd, const FieldPlotCommand::FieldSpec& fs, const miTime& const_ptime, vector<Field*>& vfout)
{
  METLIBS_LOG_SCOPE();
  vfout.clear();

  std::vector<FieldRequest> vfieldrequest;
  std::string plotName;
  parsePin(cmd, fs, vfieldrequest, plotName);

  for (FieldRequest& fr : vfieldrequest) {
    if (fr.ptime.undef())
      fr.ptime = const_ptime;

    if (fr.hourOffset != 0)
      fr.ptime.addHour(fr.hourOffset);
    if (fr.minOffset != 0)
      fr.ptime.addMin(fr.minOffset);

    Field* fout = 0;
    if (!fieldManager->makeField(fout, fr))
      return false;

    makeFieldText(fout, plotName, fr.flightlevel);
    vfout.push_back(fout);
  }

  return true;
}

void FieldPlotManager::freeFields(const std::vector<Field*>& fv)
{
  for (size_t i = 0; i < fv.size(); i++)
    fieldManager->freeField(fv[i]);
}

bool FieldPlotManager::makeDifferenceField(FieldPlotCommand_cp cmd, const miTime& const_ptime, vector<Field*>& fv)
{
  fv.clear();
  vector<Field*> fv1;
  vector<Field*> fv2;

  if (makeFields(cmd, cmd->field, const_ptime, fv1)) {
    if (!makeFields(cmd, cmd->minus, const_ptime, fv2)) {
      freeFields(fv1);
      return false;
    }
  } else {
    return false;
  }

  //make copy of fields, do not change the field cache
  for (unsigned int i = 0; i < fv1.size(); i++) {
    Field* ff = new Field();
    fv.push_back(ff);
    *fv[i] = *fv1[i];
  }

  freeFields(fv1);

  //make Difference Field text
  Field* f1 = fv[0];
  Field* f2 = fv2[0];

  const int mdiff = 6;
  std::string text1[mdiff], text2[mdiff];
  bool diff[mdiff];
  text1[0] = f1->modelName;
  text1[1] = f1->name;
  text1[2] = f1->leveltext;
  text1[3] = f1->idnumtext;
  text1[4] = f1->progtext;
  text1[5] = f1->timetext;
  text2[0] = f2->modelName;
  text2[1] = f2->name;
  text2[2] = f2->leveltext;
  text2[3] = f2->idnumtext;
  text2[4] = f2->progtext;
  text2[5] = f2->timetext;
  int nbgn = -1;
  int nend = 0;
  int ndiff = 0;
  for (int n = 0; n < mdiff; n++) {
    if (text1[n] != text2[n]) {
      diff[n] = true;
      if (nbgn < 0) {
        nbgn = n;
      }
      nend = n;
      ndiff++;
    } else {
      diff[n] = false;
    }
  }

  if (ndiff == 0) {
    // may happen due to level/idnum up/down change or equal difference,
    // make an explaining text
    if (!f1->leveltext.empty()) {
      diff[2] = true;
    } else if (!f1->idnumtext.empty()) {
      diff[3] = true;
    } else {
      diff[1] = true;
    }
    ndiff = 1;
  }

  if (diff[0])
    f1->modelName = "( " + text1[0] + " - " + text2[0] + " )";
  if (diff[1] && (diff[2] || diff[3])) {
    f1->name = "( " + text1[1];
    diutil::appendText(f1->name, text1[2]);
    diutil::appendText(f1->name, text1[3]);

    f1->name += " - " + text2[1];
    diutil::appendText(f1->name, text2[2]);
    diutil::appendText(f1->name, text2[3]);

    f1->name += " )";
    f1->leveltext.clear();
    if (diff[2] && diff[3])
      ndiff -= 2;
    else
      ndiff -= 1;
  } else {
    if (diff[1])
      f1->name = "( " + text1[1] + " - " + text2[1] + " )";
    if (diff[2])
      f1->leveltext = "( " + text1[2] + " - " + text2[2] + " )";
    if (diff[3])
      f1->idnumtext = "( " + text1[3] + " - " + text2[3] + " )";
  }
  if (diff[4])
    f1->progtext = "( " + text1[4] + " - " + text2[4] + " )";
  if (diff[5])
    f1->timetext = "( " + text1[5] + " - " + text2[5] + " )";
  if (ndiff == 1) {
    f1->fieldText = f1->modelName + " " + f1->name;
    diutil::appendText(f1->fieldText, f1->leveltext);
    f1->text = f1->fieldText + " " + f1->progtext;
    f1->fulltext = f1->text + " " + f1->timetext;
  } else {
    if (nbgn == 1 && nend <= 3) {
      diutil::appendText(text1[1], text1[2]);
      diutil::appendText(text1[1], text1[3]);
      diutil::appendText(text2[1], text2[2]);
      diutil::appendText(text2[1], text2[3]);

      text1[2].clear();
      text1[3].clear();
      text2[2].clear();
      text2[3].clear();
      nend = 1;
    }
    const int nmax[3] = { 5, 4, 3 };
    std::string ftext[3];
    for (int t = 0; t < 3; t++) {
      if (nbgn > nmax[t])
        nbgn = nmax[t];
      if (nend > nmax[t])
        nend = nmax[t];
      if (nbgn > 0) {
        ftext[t] = text1[0];
        for (int n = 1; n < nbgn; n++)
          ftext[t] += " " + text1[n];
      }
      diutil::appendText(ftext[t], "(");
      for (int n = nbgn; n <= nend; n++)
        diutil::appendText(ftext[t], text1[n]);

      ftext[t] += " -";

      for (int n = nbgn; n <= nend; n++)
        diutil::appendText(ftext[t], text2[n]);

      ftext[t] += " )";

      for (int n = nend + 1; n <= nmax[t]; n++)
        diutil::appendText(ftext[t], text1[n]);
    }
    f1->fulltext = ftext[0];
    f1->text = ftext[1];
    f1->fieldText = ftext[2];
  }

  METLIBS_LOG_DEBUG("F1-F2: validFieldTime: "<<f1->validFieldTime);
  METLIBS_LOG_DEBUG("F1-F2: analysisTime:   "<<f1->analysisTime);
  METLIBS_LOG_DEBUG("F1-F2: name:           "<<f1->name);
  METLIBS_LOG_DEBUG("F1-F2: text:           "<<f1->text);
  METLIBS_LOG_DEBUG("F1-F2: fulltext:       "<<f1->fulltext);
  METLIBS_LOG_DEBUG("F1-F2: modelName:      "<<f1->modelName);
  METLIBS_LOG_DEBUG("F1-F2: fieldText:      "<<f1->fieldText);
  METLIBS_LOG_DEBUG("F1-F2: leveltext:      "<<f1->leveltext);
  METLIBS_LOG_DEBUG("F1-F2: idnumtext:      "<<f1->idnumtext);
  METLIBS_LOG_DEBUG("F1-F2: progtext:       "<<f1->progtext);
  METLIBS_LOG_DEBUG("F1-F2: timetext:       "<<f1->timetext);
  METLIBS_LOG_DEBUG("-----------------------------------------------------");

  return fieldManager->makeDifferenceFields(fv, fv2);
}

void FieldPlotManager::getFieldPlotGroups(const std::string& modelName, const std::string& refTime, bool predefinedPlots, FieldPlotGroupInfo_v& vfgi)
{
  vfgi.clear();

  map<std::string, FieldPlotInfo> fieldInfo;
  fieldManager->getFieldPlotInfo(modelName, refTime, fieldInfo);
  map<std::string, FieldPlotGroupInfo> mfgi;

  if (!predefinedPlots) {
    for (auto&& vi : fieldInfo) {
      const FieldPlotInfo& plotInfo = vi.second;

      // add plot to FieldGroup
      FieldPlotGroupInfo& fgi = mfgi[plotInfo.groupName];
      fgi.plots.push_back(plotInfo);
    }

  } else {

    for (PlotField_p pf : vPlotField) {
      FieldPlotInfo plotInfo;

      size_t ninput = 0; // number of input fields found
      for (const auto& input : pf->input) {
        map<std::string, FieldPlotInfo>::const_iterator ip;
        if (input.is_standard_name) {
          ip = fieldInfo.begin();
          while (ip != fieldInfo.end() && ip->second.standard_name != input.name)
            ip++;
        } else {
          ip = fieldInfo.find(input.name);
        }

        if (ip == fieldInfo.end())
          break;
        if (!pf->vcoord.empty() && !pf->vcoord.count(ip->second.vcoord()))
          break;

        if ( ninput == 0) {
          plotInfo = ip->second;
          plotInfo.fieldName = pf->name;
          plotInfo.variableName = pf->inputstr;
          if (!pf->fieldgroup.empty())
            plotInfo.groupName = pf->fieldgroup;
        } else {
          if (plotInfo.vlevels() != ip->second.vlevels()) {
            // if the input parameters have different vlevels, but no. of levels are 0 or 1, ignore vlevels
            if (plotInfo.vlevels().size() < 2 && ip->second.vlevels().size() < 2) {
              plotInfo.vertical_axis.values.clear();
              plotInfo.vertical_axis.default_value_index = -1;
              plotInfo.vertical_axis.name.clear();
              } else {
                break;
              }
          }
        }
        ninput++;
      }

      if (ninput == pf->input.size()) {

        //add flightlevels
        if (plotInfo.vcoord() == "pressure") {
          FieldPlotInfo plotInfo_fl = plotInfo;
          miutil::replace(plotInfo_fl.groupName,"pressure","flightlevel");
          plotInfo_fl.vertical_axis.name = "flightlevel";
          for (size_t i = 0; i < plotInfo.vlevels().size(); ++i)
            plotInfo_fl.vertical_axis.values[i] = FlightLevel::getFlightLevel(plotInfo.vertical_axis.values[i]);
          const std::map<std::string, std::string>::const_iterator itFL = groupNames.find(plotInfo_fl.groupName);
          if (itFL != groupNames.end())
            plotInfo_fl.groupName = itFL->second;

          mfgi[plotInfo_fl.groupName].plots.push_back(plotInfo_fl);
        }

        // add plot to FieldGroup
        const std::map<std::string, std::string>::const_iterator it = groupNames.find(plotInfo.groupName);
        if (it != groupNames.end())
          plotInfo.groupName = it->second;
        mfgi[plotInfo.groupName].plots.push_back(plotInfo);
      }

    }
  }

  for (auto& name_fpgi : mfgi) {
    vfgi.push_back(name_fpgi.second);
  }
}


std::string FieldPlotManager::getBestFieldReferenceTime(const std::string& model, int refOffset, int refHour)
{
  return fieldManager->getBestReferenceTime(model, refOffset, refHour);
}

gridinventory::Grid FieldPlotManager::getFieldGrid(const std::string& model)
{
  return fieldManager->getGrid(model);
}

void FieldPlotManager::parseString(FieldPlotCommand_cp cmd, const FieldPlotCommand::FieldSpec& fs, FieldRequest& fieldrequest,
                                   std::vector<std::string>& paramNames, std::string& plotName)
{
  METLIBS_LOG_SCOPE(LOGVAL(cmd->toString()));

  fieldrequest.modelName = fs.model;
  fieldrequest.refTime = fs.reftime;
  fieldrequest.refoffset = fs.refoffset;
  fieldrequest.refhour = fs.refhour;
  if (fs.isPredefinedPlot()) {
    plotName = fs.plot;
    fieldrequest.predefinedPlot = true;
  } else {
    paramNames = fs.parameters;
    fieldrequest.predefinedPlot = false;
  }
  fieldrequest.zaxis = fs.vcoord;
  fieldrequest.plevel = fs.vlevel;
  fieldrequest.eaxis = fs.ecoord;
  fieldrequest.elevel = fs.elevel;
  fieldrequest.unit = fs.units;
  if (!cmd->time.empty())
    fieldrequest.ptime = miutil::miTime(cmd->time);
  fieldrequest.hourOffset = fs.hourOffset;
  fieldrequest.time_tolerance = fs.hourDiff * 60; // time_tolerance in minutes, hourDiff in hours

  for (const miutil::KeyValue& kv : cmd->options()) {
    if (!kv.value().empty()) {
      const std::string& key = kv.key();
      if (key == "tcoor") {
        fieldrequest.taxis = kv.value();
      } else if (key == "vunit" && kv.value() == "FL") {
        fieldrequest.flightlevel=true;
      } else if (key == "min.offset") {
        fieldrequest.minOffset = kv.toInt();
      } else if (key == "alltimesteps") {
        fieldrequest.allTimeSteps = kv.toBool();
      } else if (key == "file.palette") {
        fieldrequest.palette = kv.value();
      }
    }
  }

  flightlevel2pressure(fieldrequest);
  METLIBS_LOG_DEBUG(LOGVAL(fieldrequest.zaxis) << LOGVAL(fieldrequest.plevel));
}

void FieldPlotManager::parsePin(FieldPlotCommand_cp cmd, const FieldPlotCommand::FieldSpec& fs, std::vector<FieldRequest>& vfieldrequest, std::string& plotName)
{
  METLIBS_LOG_SCOPE(LOGVAL(cmd->toString()));

  FieldRequest fieldrequest;
  vector<std::string> paramNames;
  parseString(cmd, fs, fieldrequest, paramNames, plotName);

  // plotName -> fieldName
  if (fieldrequest.predefinedPlot) {
    vfieldrequest = getParamNames(plotName, fieldrequest);
  } else {
    vfieldrequest.reserve(paramNames.size());
    for (const std::string& pn : paramNames) {
      fieldrequest.paramName = pn;
      vfieldrequest.push_back(fieldrequest);
    }
  }
}

bool FieldPlotManager::writeField(const FieldRequest& fieldrequest, const Field* field)
{
  return fieldManager->writeField(fieldrequest, field);
}

vector<FieldRequest> FieldPlotManager::getParamNames(const std::string& plotName, FieldRequest fieldrequest)
{
  //search through vPlotField
  //if plotName and vcoord ok -> use it
  //else use fieldname= plotname

  vector<FieldRequest> vfieldrequest;

  for (PlotField_p pf : vPlotField) {
    if ((pf->name == plotName) && ((pf->vcoord.empty() && pf->vctype == FieldFunctions::vctype_none) || pf->vcoord.count(fieldrequest.zaxis) ||
                                   pf->vctype == FieldFunctions::Zaxis_info_map[fieldrequest.zaxis].vctype)) {
      for (const auto& input : pf->input) {
        fieldrequest.paramName = input.name;
        fieldrequest.standard_name = input.is_standard_name;
        vfieldrequest.push_back(fieldrequest);
      }

      return vfieldrequest;
    }
  }

  //If plotName not defined, use plotName as fieldName
  fieldrequest.paramName = plotName;
  vfieldrequest.push_back(fieldrequest);
  return vfieldrequest;
}

// ---------------- fieldPlotOptions management --------------------

// update static fieldplotoptions
bool FieldPlotManager::updateFieldPlotOptions(const std::string& name,
    const miutil::KeyValue_v& opts)
{
  return PlotOptions::parsePlotOption(opts, fieldPlotOptions[name], fieldDataOptions[name]);
}

void FieldPlotManager::getSetupFieldOptions(std::map<std::string, miutil::KeyValue_v>& fieldoptions) const
{
  // The selected PlotOptions elements are used to activate elements
  // in the FieldDialog (any remaining will be used unchanged from setup)
  // Also return any field prefixes and suffixes used.

  fieldoptions.clear();
  for (const auto& pf : vPlotField) {
    const std::string& fn = pf->name;

    miutil::KeyValue_v fdo;
    PlotOptions po;
    getFieldPlotOptions(fn, po, fdo);

    miutil::KeyValue_v& fpo = fieldoptions[fn];
    fpo = po.toKeyValueList();
    diutil::insert_all(fpo, fdo);
  }
}

void FieldPlotManager::getFieldPlotOptions(const std::string& name, PlotOptions& po, miutil::KeyValue_v& fdo) const
{
  map<std::string, PlotOptions>::const_iterator p = fieldPlotOptions.find(name);
  if (p != fieldPlotOptions.end()) {
    po = p->second;
  }
  map<std::string, miutil::KeyValue_v>::const_iterator ido = fieldDataOptions.find(name);
  if (ido != fieldDataOptions.end()) {
    fdo = ido->second;
  }
}
