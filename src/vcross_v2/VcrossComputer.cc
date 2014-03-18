
#include "VcrossComputer.h"
#include "diField/VcrossUtil.h"

#include "puTools/mi_boost_compatibility.hh"
#include "puTools/miStringFunctions.h"
#include "diField/diFieldFunctions.h"
#include "diField/diMetConstants.h"

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "vcross.Computer"
#include "miLogger/miLogging.h"

namespace { // anonymous

using namespace vcross;

const FunctionSpec vcross_functions[vcross::vcf_no_function] = {
  { "identity", 1, vcf_identity, 0, {0} },

  { "add", 2, vcf_add, 0, {0,0} },
  { "subtract", 2, vcf_subtract, 0, {0,0} },
  { "multiply", 2, vcf_multiply, 0, {0,0} },
  { "divide", 2, vcf_divide, 0, {0,0} },

  { "convert_unit", 2, vcf_convert_unit, 0, {0} },
  { "impose_unit",  2, vcf_impose_unit,  0, {0} },

  { "total", 2, vcf_total, 0, {0} },
  { "normal", 2, vcf_normal, 0, {0} },
  { "tangential", 2, vcf_tangential, 0, {0} },
  { "sqrt", 1, vcf_sqrt, 0, {0} },

  { "tk_from_th", 1, vcf_tk_from_th, "K", {"K"} },
  { "th_from_tk", 1, vcf_th_from_tk, "K", {"K"} },
  { "thesat_from_tk", 1, vcf_thesat_from_tk, "K", {"K"} },
  { "thesat_from_th", 1, vcf_thesat_from_th, "K", {"K"} },

  { "the_from_tk_q", 2, vcf_the_from_tk_q, "K", {"K", "1"} },
  { "the_from_th_q", 2, vcf_the_from_th_q, "K", {"K", "1"} },
  { "tdk_from_tk_q", 2, vcf_tdk_from_tk_q, "K", {"K", "1"} },
  { "tdk_from_th_q", 2, vcf_tdk_from_th_q, "K", {"K", "1"} },

  { "tdk_from_tk_rh", 2, vcf_tdk_from_tk_rh, "K", {"K", "1"} },
  { "tdk_from_th_rh", 2, vcf_tdk_from_th_rh, "K", {"K", "1"} },

  { "rh_from_tk_q", 2, vcf_rh_from_tk_q, "1", {"K", "1"} },
  { "rh_from_th_q", 2, vcf_rh_from_th_q, "1", {"K", "1"} },
  { "q_from_tk_rh", 2, vcf_q_from_tk_rh, "1", {"K", "1"} },
  { "q_from_th_rh", 2, vcf_q_from_th_rh, "1", {"K", "1"} },

//  { "ducting_from_tk_q", 2, vcf_ducting_from_tk_q },
//  { "ducting_from_th_q", 2, vcf_ducting_from_th_q },
//  { "ducting_from_tk_rh", 2, vcf_ducting_from_tk_rh },
//  { "ducting_from_th_rh", 2, vcf_ducting_from_th_rh },
//  { "d_ducting_dz_from_tk_q", 2, vcf_d_ducting_dz_from_tk_q },
//  { "d_ducting_dz_from_th_q", 2, vcf_d_ducting_dz_from_th_q },
//  { "d_ducting_dz_from_tk_rh", 2, vcf_d_ducting_dz_from_tk_rh },
//  { "d_ducting_dz_from_th_rh", 2, vcf_d_ducting_dz_from_th_rh },

//  { "momentum_vn_fs", 1, vcf_momentum_vn_fs },
//  { "height_above_msl_from_th", 1, vcf_height_above_msl_from_th },
//  { "height_above_surface_from_th", 1, vcf_height_above_surface_from_th }
  { "height_above_msl_from_surface_geopotential", 1, vcf_height_above_msl_from_surface_geopotential, "m", { "m^2/s^2" } }
};
const int n_vcross_functions = sizeof(vcross_functions)/sizeof(vcross_functions[0]);

const FunctionSpec* findFunctionByName(const std::string& name)
{
  for (int i=0; i<n_vcross_functions; ++i) {
    if (vcross_functions[i].name == name)
      return &vcross_functions[i];
  }
  return 0;
}

FieldFunctions ffunc; // Container for Field Functions

} // anonymous namespace 

// ========================================================================

namespace vcross {

const char VC_PRESSURE[]  = "__PRESSURE";
const char VC_LONGITUDE[] = "__LONGITUDE";
const char VC_LATITUDE[]  = "__LATITUDE";
const char VC_BEARING[]   = "__BEARING";
const char VC_STEP[]      = "__STEP";
const char VC_CORIOLIS[]  = "__CORIOLIS";

const char VC_SURFACE_PRESSURE[]  = "vc_surface_pressure";
const char VC_SURFACE_HEIGHT[]    = "vc_surface_height";
const char VC_SPECIFIC_HUMIDITY[] = "vc_specific_humidity";
const char VC_AIR_TEMPERATURE[]   = "vc_air_temperature";

// ========================================================================

FunctionData::FunctionData(std::string id, const FunctionSpec* function)
  : FieldData(id, "")
  , mFunction(function)
  , mNumericArgIndex(-1)
  , mNumericArg(0)
{
}

// ------------------------------------------------------------------------

std::string FunctionData::DATA_TYPE()
{
  return "FUNCTION";
}

// ------------------------------------------------------------------------

bool FunctionData::setArguments(const string_v& arguments, const InventoryBase_cps& content)
{
  METLIBS_LOG_SCOPE();
  bool badArgs = false;
  int arg_idx = -1;
  BOOST_FOREACH(const std::string& a, arguments) {
    arg_idx += 1;
    if (arg_idx == 1 and (function() == vcf_convert_unit or function() == vcf_impose_unit)) {
      if (badArgs or nargument() == 0) {
        METLIBS_LOG_INFO("cannot change unit of unknown argument");
        badArgs = true;
      } else if (a.empty()) {
        METLIBS_LOG_INFO("cannot change to empty unit");
        badArgs = true;
      } else if (function() == vcf_convert_unit and not util::unitsConvertible(argument(0)->unit(), a)) {
        METLIBS_LOG_INFO("cannot convert from '" << argument(0)->unit() << "' to '" << a << "'");
        badArgs = true;
      } else {
        setUnit(a);
      }
    } else if (not hasNumericArg()
        and (function() == vcf_add or function() == vcf_subtract
            or function() == vcf_multiply or function() == vcf_divide)
        and a.size() >= 7 and a.compare(0, 6, "const:") == 0)
    {
      mNumericArgIndex = arg_idx;
      mNumericArg = miutil::to_double(a.substr(6));
    } else if (InventoryBase_cp it = findItemById(content, a)) {
      const std::string &a_unit = it->unit();
      const char* f_unit = mFunction->unit_args[arg_idx];
      if (f_unit) {
        if (not util::unitsIdentical(a_unit, f_unit)) {
          METLIBS_LOG_INFO("argument unit must be '" << f_unit << "', not '" << a_unit << "'");
          badArgs = true;
        }
      } else if (arg_idx >= 1 and function() != vcf_multiply and function() != vcf_divide) {
        const std::string &a0_unit = argument(0)->unit();
        if (not util::unitsIdentical(a0_unit, a_unit)) {
          METLIBS_LOG_INFO("argument units '" << a0_unit << "' and '" << a_unit << "' not identical");
          badArgs = true;
        }
      } else if (function() == vcf_sqrt) {
        setUnit(util::unitsRoot(a_unit, 2));
      }
      // TODO check that the z axes are identical
      addArgument(it);
    } else {
      METLIBS_LOG_WARN("argument '" << a << "' not known");
      badArgs = true;
    }
  }
  if (badArgs)
    return false;

  METLIBS_LOG_DEBUG(LOGVAL(nargument()) << LOGVAL(unit()));
  if (nargument() > 0) {
    if (unit().empty()) {
      const char* o_unit = mFunction->unit_out;
      if (o_unit)
        setUnit(o_unit);
      else if ((function() == vcf_multiply or function() == vcf_divide) and not hasNumericArg())
        setUnit(util::unitsMultiplyDivide(argument(0)->unit(), argument(1)->unit(), function() == vcf_multiply));
      else
        setUnit(argument(0)->unit());
    }
    FieldData_cp fa0 = boost::dynamic_pointer_cast<const FieldData>(argument(0));
    if (fa0)
      setZAxis(fa0->zaxis());
  }
  return true;
}

// ------------------------------------------------------------------------

Values_cp FunctionData::evaluate(name2value_t& n2v) const
{
  METLIBS_LOG_SCOPE(LOGVAL(id()) << LOGVAL(mFunction->name));
  Values_cpv argument_values;
  int arg_idx = -1;
  BOOST_FOREACH(const InventoryBase_cp& a, arguments()) {
    arg_idx += 1;
    if (arg_idx == mNumericArgIndex
        or (function() == vcf_convert_unit and arg_idx == 1))
    {
      continue;
    }
    name2value_t::const_iterator ita = n2v.find(a->id());
    if (ita == n2v.end()) {
      // argument values missing
      if (not vcross::vc_evaluate_field(a, n2v)) {
        METLIBS_LOG_WARN("argument '" << a->id() << "' has no values");
        return Values_p();
      }
      ita = n2v.find(a->id());
      if (ita == n2v.end()) {
        METLIBS_LOG_WARN("argument '" << a->id() << "' has no values after evaluating it");
        return Values_p();
      }
    }
    if (not ita->second)
      return Values_p();
   
    argument_values.push_back(ita->second);
  }

  const Values_cp av0 = argument_values[0], av1 = (argument_values.size() >= 2 ? argument_values[1] : Values_cp());
  const float ud0 = av0->undefValue(), ud1 = (av1 ? av1->undefValue() : ud0);
  const size_t np = av0->npoint(), nl = av0->nlevel();
  Values_p out = Values_p(new Values(np, nl));

  const float *f0 = av0->values().get(), *f1 = (av1 ? av1->values().get() : 0);
  float* fo = out->values().get();
  bool allDefined = false;

  int compute = 0;
  switch(function()) {
  case vcf_identity:
    return argument_values[0];

  case vcf_add:
    compute = 1;
  case vcf_subtract:
    if (compute == 0) compute = 2;
  case vcf_multiply:
    if (compute == 0) compute = 3;
  case vcf_divide: {
    if (compute == 0) compute = 4;

    if (mNumericArgIndex < 0) {
      if (!ffunc.fieldOPERfield(compute, np, nl, f0, f1, fo, allDefined, ud0))
        return Values_p();
    } else if (mNumericArgIndex == 0) {
      if (!ffunc.constantOPERfield(compute, np, nl, mNumericArg, f0, fo, allDefined, ud0))
        return Values_p();
    } else if (mNumericArgIndex == 1) {
      if (!ffunc.fieldOPERconstant(compute, np, nl, f0, mNumericArg, fo, allDefined, ud0))
        return Values_p();
    } else {
      return Values_p();
    }
    break; }

  case vcf_convert_unit:
    return util::unitConversion(av0, argument(0)->unit(), unit());

  case vcf_impose_unit:
    return av0;

  case vcf_normal: case vcf_tangential: {
    const name2value_t::const_iterator itb = n2v.find(VC_BEARING);
    if (itb == n2v.end())
      return Values_p();
    const Values_cp vbearing = itb->second;

    const bool ff_normal = (function() == vcf_normal);
    for (size_t p = 0; p < np; p++) {
      const detail::NormalTangential func(ff_normal, vbearing->value(p, 0));
      for (size_t l = 0; l < nl; l++) {
        const float pX = av0->value(p, l), pY = av1->value(p, l);
        const float v = (pX != ud0 and pY != ud1) ? func(pX, pY): ud0;
        out->setValue(v, p, l);
      }
    }
    break; }

  case vcf_total: {
    for (size_t l = 0; l < nl; l++) {
      for (size_t p = 0; p < np; p++) {
        const float v0 = av0->value(p, l), v1 = av1->value(p, l);
        const float v = (v0 != ud0 and v1 != ud1) ? std::sqrt(v0*v0 + v1*v1) : ud0;
        out->setValue(v, p, l);
      }
    }
    break; }

  case vcf_sqrt: {
    std::transform(f0, f0+nl*np, out->values().get(), sqrt);
    break; }

  case vcf_tk_from_th:
    if (compute == 0) compute = 2;
  case vcf_th_from_tk:
    if (compute == 0) compute = 3;
  case vcf_thesat_from_tk:
    if (compute == 0) compute = 4;
  case vcf_thesat_from_th: {
    if (compute == 0) compute = 5;

    name2value_t::const_iterator itpp = n2v.find(VC_PRESSURE);
    if (itpp == n2v.end())
      return Values_cp();
    const float* f_pp = itpp->second->values().get();
    if (not f_pp)
      return Values_cp();

    if (not ffunc.aleveltemp(compute, np, nl, f0, f_pp, fo, allDefined, ud0, "kelvin"))
      return Values_cp();
    break; }

  case vcf_the_from_tk_q:
    if (compute == 0) compute = 1;
  case vcf_the_from_th_q: {
    if (compute == 0) compute = 2;

    name2value_t::const_iterator itpp = n2v.find(VC_PRESSURE);
    if (itpp == n2v.end())
      return Values_cp();
    const float* f_pp = itpp->second->values().get();
    if (not f_pp)
      return Values_cp();

    if (not ffunc.alevelthe(compute, np, nl, f0, f1, f_pp, fo, allDefined, ud0))
      return Values_cp();
    break; }

  case vcf_rh_from_tk_q:
    if (compute == 0)
      compute = 1;
  case vcf_rh_from_th_q:
    if (compute == 0)
      compute = 2;
  case vcf_tdk_from_tk_q:
    if (compute == 0)
      compute = 9;
  case vcf_tdk_from_th_q:
    if (compute == 0)
      compute = 10;
  case vcf_q_from_tk_rh:
    if (compute == 0)
      compute = 3;
  case vcf_q_from_th_rh:
    if (compute == 0)
      compute = 4;
  case vcf_tdk_from_tk_rh:
    if (compute == 0)
      compute = 11;
  case vcf_tdk_from_th_rh: {
    if (compute == 0)
      compute = 12;

    name2value_t::const_iterator itpp = n2v.find(VC_PRESSURE);
    if (itpp == n2v.end())
      return Values_p();
    const float* f_pp = itpp->second->values().get();
    if (not f_pp)
      return Values_p();

    if (not ffunc.alevelhum(compute, np, nl, f0, f1, f_pp, fo, allDefined, ud0, "kelvin"))
      return Values_p();
    break; }

  case vcf_height_above_msl_from_surface_geopotential: {
    compute = 3;
    if (!ffunc.fieldOPERconstant(compute, out->npoint(), out->nlevel(), f0, MetNo::Constants::ginv, fo, allDefined, ud0))
      return Values_p();
    break; }

  case vcf_no_function:
    return Values_p();
  }
  return out;
}

// ========================================================================

void resolve(InventoryBase_cps& content, const NameItem_v& nameItems)
{
  METLIBS_LOG_SCOPE();
  BOOST_FOREACH(const NameItem& ni, nameItems) {
    if (findItemById(content, ni.name)) {
      METLIBS_LOG_DEBUG("name '" << ni.name << "' already defined");
      continue;
    }
    const FunctionSpec* function = findFunctionByName(ni.function);
    if (not function) {
      METLIBS_LOG_WARN("function '" << ni.function << "' not known");
      continue;
    }
    if (ni.arguments.size() != function->argc) {
      METLIBS_LOG_WARN("function '" << ni.function << "' expects " << function->argc << " arguments");
      continue;
    }

    FunctionData_p f(new FunctionData(ni.name, function));
    if (not f->setArguments(ni.arguments, content))
      continue;

    METLIBS_LOG_DEBUG("adding '" << ni.name << "'");
    content.insert(f);
  }
}

// ================================================================================

void resolveCrossection(InventoryBase_cps& inv)
{
  inv.insert(FieldData_p(new FieldData(VC_LONGITUDE, "rad")));
  inv.insert(FieldData_p(new FieldData(VC_LATITUDE,  "rad")));
  inv.insert(FieldData_p(new FieldData(VC_BEARING,   "rad")));
  inv.insert(FieldData_p(new FieldData(VC_STEP,      "km")));
  inv.insert(FieldData_p(new FieldData(VC_CORIOLIS,  "1")));
}

namespace /*anonymous*/ {
void evaluateCrossectionPoint(Crossection_cp cs, size_t cs_index,
    Values::value_t& vlon, Values::value_t& vlat, Values::value_t& vbrng, Values::value_t& vstep, Values::value_t& vcor)
{
  if (cs_index >= cs->npoint())
    return;

  const LonLat &p0 = cs->point(cs_index);
  vlon = p0.lon();
  vlat = p0.lat();
  if (cs->npoint() == 1)
    vbrng = 0;
  else if (cs_index < cs->npoint()-1)
    vbrng = p0.bearingTo(cs->point(cs_index+1));
  else // cs_index == cs->npoint()-1 and size >= 2
    // => repeat bearing of last segment
    vbrng = cs->point(cs_index-1).bearingTo(p0);
  if (cs_index == 0)
    vstep = 0;
  else
    vstep = cs->point(cs_index-1).distanceTo(p0);
  vcor = util::coriolisFactor(p0.lat());
}
} // namespace anonymous

void evaluateCrossection(Crossection_cp cs, name2value_t& n2v)
{
  Values_p vlon (new Values(cs->npoint(), 1, false));
  Values_p vlat (new Values(cs->npoint(), 1, false));
  Values_p vbrng(new Values(cs->npoint(), 1, false));
  Values_p vstep(new Values(cs->npoint(), 1, false));
  Values_p vcor (new Values(cs->npoint(), 1, false));
  
  for (size_t i=0; i<cs->npoint(); ++i) {
    Values::value_t flon, flat, fbrng, fstep, fcor;
    evaluateCrossectionPoint(cs, i, flon, flat, fbrng, fstep, fcor);
    vlon ->setValue(flon,  i, 0);
    vlat ->setValue(flat,  i, 0);
    vbrng->setValue(fbrng, i, 0);
    vstep->setValue(fstep, i, 0);
    vcor ->setValue(fcor,  i, 0);
  }

  n2v[VC_LONGITUDE] = vlon; 
  n2v[VC_LATITUDE]  = vlat;
  n2v[VC_BEARING]   = vbrng;
  n2v[VC_STEP]      = vstep;
  n2v[VC_CORIOLIS]  = vcor;
}

void evaluateCrossection4TimeGraph(Crossection_cp cs, size_t cs_index, size_t ntimes, name2value_t& n2v)
{
  if (cs_index >= cs->npoint())
    return;

  Values::value_t flon, flat, fbrng, fstep, fcor;
  evaluateCrossectionPoint(cs, cs_index, flon, flat, fbrng, fstep, fcor);

  Values_p vlon (new Values(ntimes, 1, false));
  Values_p vlat (new Values(ntimes, 1, false));
  Values_p vbrng(new Values(ntimes, 1, false));
  Values_p vstep(new Values(ntimes, 1, false));
  Values_p vcor (new Values(ntimes, 1, false));

  for (size_t i=0; i<ntimes; ++i) {
    vlon ->setValue(flon,  i, 0);
    vlat ->setValue(flat,  i, 0);
    vbrng->setValue(fbrng, i, 0);
    vstep->setValue(fstep, i, 0);
    vcor ->setValue(fcor,  i, 0);
  }

  n2v[VC_LONGITUDE] = vlon; 
  n2v[VC_LATITUDE]  = vlat;
  n2v[VC_BEARING]   = vbrng;
  n2v[VC_STEP]      = vstep;
  n2v[VC_CORIOLIS]  = vcor;
}

// ================================================================================

void collectRequired(InventoryBase_cps& required, InventoryBase_cp item)
{
  //METLIBS_LOG_SCOPE();
  if (item->dataType() == FieldData::DATA_TYPE()) {
    //METLIBS_LOG_DEBUG("field, inserting '" << item->id() << "'");
    required.insert(item);
  } else if (item->dataType() == FunctionData::DATA_TYPE()) {
    FunctionData_cp f = boost::static_pointer_cast<const FunctionData>(item);
    BOOST_FOREACH(const InventoryBase_cp& a, f->arguments()) {
      collectRequired(required, a);
    }
  }
}

// ================================================================================

Values_cp vc_evaluate_field(InventoryBase_cp item, name2value_t& n2v)
{
  METLIBS_LOG_SCOPE();
  if (not item)
    return Values_cp();

  METLIBS_LOG_DEBUG(LOGVAL(item->id()));
  name2value_t::iterator it = n2v.find(item->id());
  if (it != n2v.end())
    return it->second;

  // insert an item to protect against recursion
  Values_cp& out = n2v[item->id()];

  if (item->dataType() != FunctionData::DATA_TYPE()) {
    // no idea how to evaluate other things than functions
    METLIBS_LOG_WARN("cannot evaluate '" << item->id() << "' with type '" << item->dataType() << "'");
    return Values_cp();
  }

  FunctionData_cp f = boost::static_pointer_cast<const FunctionData>(item);
  out = f->evaluate(n2v);
  return out;
}

// ================================================================================

Values_cp heightFromPressure(Values_cp pressure, bool positiveUp, Values_cp specific_humidity, Values_cp air_temperature,
    Values_cp surface_pressure, Values_cp topography)
{
  METLIBS_LOG_SCOPE(LOGVAL(positiveUp));
  Values_p out;
  if (not (pressure and surface_pressure and specific_humidity and air_temperature))
    return out;
  const size_t np = pressure->npoint(), nl = pressure->nlevel();
  if (surface_pressure->npoint() != np or surface_pressure->nlevel() != 1)
    return out;
  if (specific_humidity->npoint() != np or specific_humidity->nlevel() != nl)
    return out;
  if (air_temperature->npoint() != np or air_temperature->nlevel() != nl)
    return out;
#if 0
  if (specific_humidity->unit() != "1" and specific_humidity->unit() != "kg/kg")
    return out;
  if (air_temperature->unit() != "K")
    return out;
  if (pressure->unit() != surface_pressure->unit())
    return out;
  if (topography and topography->unit() != "m")
    return out;
#endif

  const int l0 = (positiveUp ? 0 : nl-1), l1 = (positiveUp ? nl-1 : 0), dl = (positiveUp ? 1 : -1);
  out = miutil::make_shared<Values>(np, nl);
  for (size_t p = 0; p < np; p++) {
    float h = (topography ? topography->value(p, 0) : 0);
    METLIBS_LOG_DEBUG(LOGVAL(p) << LOGVAL(h));
    for (int l = l0; l != l1; l += dl) {
      const float p_low_alti = (l == l0)
          ? surface_pressure->value(p, 0) : pressure->value(p, l - dl);
      const float p_high_alti = pressure->value(p, l);
      const float dh = util::heightDifferenceFromPressureDifference(p_low_alti, p_high_alti,
          specific_humidity->value(p, l), air_temperature->value(p, l));
      h += dh;
      if (p < 10)
        METLIBS_LOG_DEBUG(LOGVAL(p) << LOGVAL(l) << LOGVAL(p_low_alti) << LOGVAL(p_high_alti) << LOGVAL(dh) << LOGVAL(h));
      out->setValue(h, p, l);
    }
  }
  return out;
}

// ================================================================================

Values_cp potentialTemperature(Values_cp pressure, Values_cp air_temperature)
{
  Values_p out;
  if (not (pressure and air_temperature))
    return out;
  const size_t np = pressure->npoint(), nl = pressure->nlevel();
  if (air_temperature->npoint() != np or air_temperature->nlevel() != nl)
    return out;

  float p0 = 1000; // hPa
#if 0
  if (pressure->unit() == "Pa")
    p0 *= 100;
  else if (pressure->unit() != "hPa")
    return out;
  if (air_temperature->unit() != "K")
    return out;
#endif

  out = miutil::make_shared<Values>(np, nl); // in K
  for (size_t p = 0; p < np; p++) {
    for (size_t l = 0; l < nl; l++) {
      out->setValue(util::potentialTemperature(p0, pressure->value(p, l), air_temperature->value(p, l)), p, l);
    }
  }
  return out;
}

// ========================================================================

namespace detail {

NormalTangential::NormalTangential(bool normal, float vb)
{
  const float b_offset = (90*DEG_TO_RAD * (normal ? 2 : 1));
  const float b = b_offset - vb;
  bsin = std::sin(b);
  bcos = std::cos(b);
}

} // namespace detail

} // namespace vcross