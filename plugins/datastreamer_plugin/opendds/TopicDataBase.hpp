// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima OpenDDS Visualizer Plugin.
//
// eProsima OpenDDS Visualizer Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima OpenDDS Visualizer Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima OpenDDS Visualizer Plugin. If not, see <https://www.gnu.org/licenses/>.

/**
 * @file TopicDataBase.hpp
 */

#ifndef _EPROSIMA_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_OPENDDS_TOPICDATABASE_HPP_
#define _EPROSIMA_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_OPENDDS_TOPICDATABASE_HPP_

#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <dds/DdsDcpsCoreTypeSupportC.h>

namespace eprosima {
namespace plotjuggler {
namespace opendds {

/**
 * @brief Type aliases for topic and data type information storage.
 *
 * These aliases define the data structures used to track discovered topics
 * and their associated data types in the OpenDDS domain.
 */
using TopicName = std::string;
using DataTypeNameType = std::string;
using DataTypeId = OpenDDS::XTypes::TypeIdentifier;
using TypeInfoAvailable = bool;
using DataTypeRegistryInfo = std::pair<DataTypeNameType, TypeInfoAvailable>;
using DataTypeIdInfo = std::pair<DataTypeNameType, DataTypeId>;
using TopicDataBase = std::unordered_map<TopicName, DataTypeRegistryInfo>;
// TopicIds stores a mapping from topic name to type name
// This is used to cache the relationship between topics and their types
using TopicIds = std::unordered_map<TopicName, DataTypeNameType>;
// TypeCache stores registered type names for quick lookup
using TypeCache = std::unordered_set<DataTypeNameType>;

} /* namespace opendds */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_OPENDDS_TOPICDATABASE_HPP_
