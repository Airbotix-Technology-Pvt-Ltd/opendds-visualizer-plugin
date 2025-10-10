// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (Airbotix).
//
// This file is part of Airbotix OpenDDS Visualizer Plugin.
//
// Airbotix OpenDDS Visualizer Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Airbotix OpenDDS Visualizer Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Airbotix OpenDDS Visualizer Plugin. If not, see <https://www.gnu.org/licenses/>.

/**
 * @file Configuration.hpp
 */

#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UTILS_DATATYPECONFIGURATION_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UTILS_DATATYPECONFIGURATION_HPP_

#include <QStringList>
#include <QSettings>
#include <QDomDocument>

namespace airbotix {
namespace plotjuggler {

/**
 * @brief This class handles every OpenDDS entity required.
 *
 * It create, manage and destroy every OpenDDS entity that the process requires to instantiate.
 * The discovery and user data received is transmitted through a UiListener object.
 *
 * FUTURE WORK:
 * Use a specific thread to call callbacks instead of using OpenDDS thread
 */
struct DataTypeConfiguration
{
    unsigned max_array_size = 1000;
    bool use_header_stamp = true;
    bool discard_large_arrays = false;
};

} /* namespace plotjuggler */
} /* namespace airbotix */

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UTILS_DATATYPECONFIGURATION_HPP_