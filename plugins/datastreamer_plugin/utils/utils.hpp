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
 * @file utils.hpp
 */

#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_UTILS_UTILS_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_UTILS_UTILS_HPP_

#include <codecvt>
#include <iostream>
#include <locale>
#include <QString>
#include <QStringList>
#include <regex>
#include <string>
#include <dds/DCPS/TimeTypes.h> // for DDS::Time_t

namespace airbotix {
namespace plotjuggler {
namespace utils {

#ifdef NDEBUG
// nondebug
    #define DEBUG(x)
    #define WARNING(x)
#else // NDEBUG
// debug code
    #define DEBUG(x) std::cout << " --- " << x << std::endl
    #define WARNING(x) std::cout << " !!! " << x << std::endl
#endif // ifdef NDEBUG

std::string get_timestamp_string(const DDS::Time_t& ts);

double get_timestamp_seconds_numeric_value(const DDS::Time_t& ts);

std::string QString_to_string(
        const QString& str);

QString string_to_QString(
        const std::string& str);

std::string to_string(
        const std::wstring& str);

std::string to_string(
        char c);

std::string to_string(
        wchar_t c);

std::vector<std::string> get_files_in_dir(
        const std::string& dir_path,
        const std::string& file_extension = "xml",
        bool recursive = false);

std::vector<std::string> get_files_in_dir_regex(
        const std::string& dir_path,
        const std::regex& regex_rule,
        bool recursive = false);

QStringList get_files_in_dir(
        const QString& dir_path,
        const std::string& file_extension = "xml",
        bool recursive = false);

} /* namespace utils */
} /* namespace plotjuggler */
} /* namespace airbotix */

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_UTILS_UTILS_HPP_