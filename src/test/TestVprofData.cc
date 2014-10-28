
#include <vcross_v2/VcrossEvaluate.h>
#include <vcross_v2/VcrossComputer.h>

#include <diField/VcrossUtil.h>

#include <puTools/mi_boost_compatibility.hh>
#include <puTools/miTime.h>

#include <gtest/gtest.h>

#include <fstream>
#include <string>

#define MILOGGER_CATEGORY "diField.test.VprofDataTest"
#include "miLogger/miLogging.h"

using namespace vcross;

//#define DEBUG_MESSAGES
#ifdef DEBUG_MESSAGES
#include <log4cpp/Category.hh>
#define configureLogging()                                              \
  milogger::LoggingConfig lc("kjlhlkjH");                               \
  log4cpp::Category::getRoot().setPriority(log4cpp::Priority::DEBUG)
#else
#define configureLogging() /* empty */
#endif // !DEBUG_MESSAGES

static const char AROME_FILE[] = "arome_vprof.nc";
static const int AROME_N_CS = 6;
static const int AROME_CS_LEN[AROME_N_CS] = { 14, 14, 14, 1, 1 };
static const int AROME_N_TIME = 2;
static const int AROME_N_Z = 65;

static const char modelName[] = "testmodel";

TEST(VprofDataTest, TestSetup)
{
  configureLogging();

  Setup_p setup = miutil::make_shared<vcross::Setup>();
  Collector_p collector = miutil::make_shared<Collector>(setup);

  string_v sources;
  sources.push_back("m=" + std::string(modelName)
      + " f=" + std::string(TOP_SRCDIR) + "/src/test/" + std::string(AROME_FILE)
      + " t=netcdf");
  EXPECT_EQ(0, setup->configureSources(sources).size()) << "syntax errors in sources";
  EXPECT_TRUE(setup->findSource(modelName));

  //parameters and computations should be defined in setup
  string_v computations;
  computations.push_back("relative_humidity_ml=rh_from_tk_q(air_temperature_ml,specific_humidity_ml)");
  computations.push_back("tdk=tdk_from_tk_q(air_temperature_ml,specific_humidity_ml)");
  computations.push_back("air_temperature_celsius_ml=convert_unit(air_temperature_ml,celsius)");
  computations.push_back("dew_point_temperature_celsius_ml=convert_unit(tdk,celsius)");
  EXPECT_EQ(0, setup->configureComputations(computations).size()) << "syntax errors in computations";
  collector->setupChanged();

  vcross::Inventory_cp inv = collector->getResolver()->getInventory(modelName);
  ASSERT_TRUE(inv);
  ASSERT_EQ(AROME_N_TIME, inv->times.npoint());
  ASSERT_EQ(AROME_N_CS, inv->crossections.size());
  Crossection_cp cs = inv->crossections.at(3);
  ASSERT_EQ(1, cs->npoint());

  string_v field_ids;
  field_ids.push_back("air_temperature_celsius_ml");
  field_ids.push_back("dew_point_temperature_celsius_ml");
  field_ids.push_back("x_wind_ml");
  field_ids.push_back("y_wind_ml");
  field_ids.push_back("relative_humidity_ml");
  field_ids.push_back("upward_air_velocity_ml");
  for (string_v::const_iterator it = field_ids.begin(); it != field_ids.end(); ++it)
    collector->requireField(modelName, *it);

  FieldData_cp air_temperature = boost::dynamic_pointer_cast<const FieldData>(collector->getResolvedField(modelName, field_ids[0]));
  ASSERT_TRUE(air_temperature);
  InventoryBase_cp zaxis = air_temperature->zaxis();
  ASSERT_TRUE(zaxis);
  collector->requireField(modelName, zaxis);

  model_values_m model_values = vc_fetch_pointValues(collector, cs->point(0), inv->times.at(0));
  model_values_m::iterator itM = model_values.find(modelName);
  ASSERT_TRUE(itM != model_values.end());
  name2value_t& n2v = itM->second;

  Values_cp zvalues = util::unitConversion(vc_evaluate_field(zaxis, n2v), zaxis->unit(), "hPa");
  ASSERT_TRUE(zvalues);
  n2v[VC_PRESSURE] = zvalues;

  vc_evaluate_fields(collector, model_values, modelName, field_ids);

  { name2value_t::const_iterator itN = n2v.find("x_wind_ml");
    EXPECT_TRUE(itN != n2v.end());
    Values_cp xwind_values = itN->second;
    ASSERT_TRUE(xwind_values);
    EXPECT_EQ(1, xwind_values->shape().rank());
  }

  { name2value_t::const_iterator itN = n2v.find("air_temperature_celsius_ml");
    EXPECT_TRUE(itN != n2v.end());
    Values_cp t_values = itN->second;
    ASSERT_TRUE(t_values);
    EXPECT_EQ(1, t_values->shape().rank());
  }

  { name2value_t::const_iterator itN = n2v.find("tdk");
    EXPECT_TRUE(itN != n2v.end());
    Values_cp tdk_values = itN->second;
    ASSERT_TRUE(tdk_values);
    EXPECT_EQ(1, tdk_values->shape().rank());
  }
}