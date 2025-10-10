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

#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UI_TOPICSELECTIONDIALOG_CONFIGURATION_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UI_TOPICSELECTIONDIALOG_CONFIGURATION_HPP_

#include <QStringList>
#include <QSettings>
#include <QDomDocument>

#include "utils/DataTypeConfiguration.hpp"

namespace airbotix {
namespace plotjuggler {
namespace ui {

/**
 * @brief This is a data structure to store the configuration of the plugin.
 */
struct Configuration
{

    Configuration();

    //! Load default configuration with specific prefix
    Configuration(
            const QString& prefix);

    Configuration(
            const Configuration& other) = default;
    Configuration& operator =(
            const Configuration& other) = default;

    ////////////////////
    // Topics
    QStringList topics_selected;  // Empty in initialization

    ////////////////////
    // Max array size
    DataTypeConfiguration data_type_configuration;

    ////////////////////
    // DDS Configuration
    unsigned int domain_id = 0;
    QStringList xml_datatypes_files;  // Empty in initialization

    ////////////////////
    // Advance options
    bool boolean_strings_to_number = false;

    ////////////////////
    // Save Load configuration
    bool xmlSaveState(
            QDomDocument& doc,
            QDomElement& parent_element) const;
    bool xmlLoadState(
            const QDomElement& parent_element);

    void save_default_settings(
            const QString& prefix) const;

    void load_default_settings(
            const QString& prefix);

protected:

    constexpr static const char* MAX_ARRAY_SIZE_SETTINGS_TAG = "max_array_size";
    constexpr static const char* USE_HEADER_STAMP_SETTINGS_TAG = "use_header_stamp";
    constexpr static const char* DISCARD_LARGE_ARRAYS_SETTINGS_TAG = "discard_large_arrays";
    constexpr static const char* XML_DATATYPE_FILES_SETTINGS_TAG = "xml_datatype_files";
    constexpr static const char* DOMAIN_ID_SETTINGS_TAG = "domain_id_files";
};


} /* namespace ui */
} /* namespace plotjuggler */
} /* namespace airbotix */

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_UI_TOPICSELECTIONDIALOG_CONFIGURATION_HPP_