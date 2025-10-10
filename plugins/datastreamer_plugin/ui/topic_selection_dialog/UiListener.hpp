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
 * @file UiListener.hpp
 */

#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UI_TOPICSELECTIONDIALOG_LISTENER_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UI_TOPICSELECTIONDIALOG_LISTENER_HPP_

#include <QObject>

#include "utils/utils.hpp"

namespace airbotix {
namespace plotjuggler {
namespace ui {

/**
 * @brief TODO
 */
struct UiListener
{
public:

    virtual void on_xml_datatype_file_added(
            const std::string& file_path)
    {
        DEBUG("Calling on_xml_datatype_file_added " << file_path);
        static_cast<void>(file_path);
    }

    virtual void on_domain_connection(
            unsigned int domain_id)
    {
        DEBUG("Calling on_domain_connection " << domain_id);
        static_cast<void>(domain_id);
    }

};

} /* namespace ui */
} /* namespace plotjuggler */
} /* namespace airbotix */

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UI_TOPICSELECTIONDIALOG_LISTENER_HPP_