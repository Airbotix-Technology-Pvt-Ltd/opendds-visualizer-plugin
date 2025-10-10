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
 * @file OpenDdsListener.hpp
 */

#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_OPENDDS_LISTENER_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_OPENDDS_LISTENER_HPP_

#include <string>

#include "utils/types.hpp"
#include "utils/utils.hpp"
#include "utils/Logger.hpp"


namespace airbotix {
namespace plotjuggler {
namespace opendds {

/**
 * @brief This class transmit callbacks that arrive from OpenDDS entities to the GUI.
 *
 * Every new discovery and every new data received from OpenDDS entities is transmitted by calling
 * one of the methods, that must be implemented by the user of the class.
 */
class OpenDdsListener
{
public:

    virtual void on_data_available()
    {
        DDS_DEBUG("OpenDdsListener","Calling on_data_available");
    }

    virtual void on_double_data_read(
            const std::vector<types::NumericDatum>& numeric_data,
            double timestamp)
    {
        DEBUG("Calling on_double_data_read");
        static_cast<void>(numeric_data);
        static_cast<void>(timestamp);
    }

    virtual void on_string_data_read(
            const std::vector<types::TextDatum>& text_data,
            double timestamp)
    {
        DEBUG("Calling on_string_data_read");
        static_cast<void>(text_data);
        static_cast<void>(timestamp);
    }

    virtual void on_topic_discovery(
            const std::string& topic_name,
            const std::string& type_name)
    {
        DDS_DEBUG("OpenDdsListener","Calling on_topic_discovery");
        static_cast<void>(topic_name);
        static_cast<void>(type_name);
    }

};

} /* namespace opendds */
} /* namespace plotjuggler */
} /* namespace airbotix */

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_OPENDDS_LISTENER_HPP_