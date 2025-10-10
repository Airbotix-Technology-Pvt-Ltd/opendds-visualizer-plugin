#ifndef PARSERS_H
#define PARSERS_H

#include <nlohmann/json.hpp>
#include <dds/DdsDcpsCoreC.h>
#include <dds/DdsDynamicDataC.h>
#include <memory>
#include "opendds/open_dynamic_data.h"

nlohmann::json parse_dynamic_data(const DDS::DynamicData_var& data);
nlohmann::json parse_dynamic_data(const std::shared_ptr<OpenDynamicData>& data);

#endif // PARSERS_H
