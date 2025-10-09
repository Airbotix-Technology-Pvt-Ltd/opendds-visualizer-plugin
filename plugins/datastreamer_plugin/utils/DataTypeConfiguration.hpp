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
 * @file Configuration.hpp
 */

#ifndef _EPROSIMA_PLOTJUGGLEROPENDDS_PLUGIN_PLUGINS_UTILS_DATATYPECONFIGURATION_HPP_
#define _EPROSIMA_PLOTJUGGLEROPENDDS_PLUGIN_PLUGINS_UTILS_DATATYPECONFIGURATION_HPP_

#include <QStringList>
#include <QSettings>
#include <QDomDocument>

namespace eprosima {
namespace plotjuggler {

/**
 * @brief This struct holds configuration for data type handling.
 *
 * It controls how data types are processed and managed by the OpenDDS visualizer plugin.
 * Settings include array size limits and timestamp usage.
 */
struct DataTypeConfiguration
{
    unsigned max_array_size = 1000;
    bool use_header_stamp = true;
    bool discard_large_arrays = false;
};

} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLEROPENDDS_PLUGIN_PLUGINS_UTILS_DATATYPECONFIGURATION_HPP_
