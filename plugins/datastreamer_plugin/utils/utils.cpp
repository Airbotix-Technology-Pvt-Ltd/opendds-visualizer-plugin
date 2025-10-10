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
 * @file utils.cpp
 */

#include <filesystem>
#include <sstream>

#include "utils.hpp"

namespace airbotix {
namespace plotjuggler {
namespace utils {

std::string get_timestamp_string(const DDS::Time_t& ts)
{
    std::time_t sec = ts.sec;
    uint32_t nanosec = ts.nanosec;
    
    std::tm tm_utc;
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_utc, &sec);
#else
    gmtime_r(&sec, &tm_utc);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << (nanosec / 1000000);
    
    return oss.str();
}

double get_timestamp_seconds_numeric_value(const DDS::Time_t& ts) {
    // Convert seconds and nanoseconds to double
    // Same formula as get_timestamp_seconds_numeric_value
    return (ts.sec + (ts.nanosec * 1e-9));
}

std::string QString_to_string(
        const QString& str)
{
    return str.toStdString();
}

QString string_to_QString(
        const std::string& str)
{
    // return QString::fromStdString(str);
    return QString::fromUtf8( str.data(), str.size());
}

std::string to_string(
        const std::wstring& str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(str);
}

std::string to_string(
        char c)
{
    return std::string(1, c);
}

std::string to_string(
        wchar_t c)
{
    return to_string(std::wstring(1, c));
}

std::vector<std::string> get_files_in_dir(
        const std::string& dir_path,
        const std::string& file_extension /* = ".xml" */,
        bool recursive /* = false */)
{
    return get_files_in_dir_regex(
        dir_path,
        std::regex(".*\\."
 + file_extension + ".*$ "),
        recursive);
}

std::vector<std::string> get_files_in_dir_regex(
        const std::string& dir_path,
        const std::regex& regex_rule,
        bool recursive /* = false */)
{
    std::vector<std::string> result;

    // Get all files in dir
    for (const auto& entry : std::filesystem::directory_iterator(dir_path))
    {
        const auto& file_path = entry.path().string();

        // For each file found, check it follows the pattern
        bool match = std::regex_match(file_path, regex_rule);
        if (match)
        {
            // If it matches, add it to the list
            result.push_back(file_path);
        }
    }

    return result;
}

QStringList get_files_in_dir(
        const QString& dir_path,
        const std::string& file_extension /* = "xml" */,
        bool recursive /* = false */)
{
    QStringList result;
    for (const auto& file : get_files_in_dir(
                QString_to_string(dir_path),
                file_extension,
                recursive))
    {
        result.append(string_to_QString(file));
    }

    return result;
}

} /* namespace utils */
} /* namespace plotjuggler */
} /* namespace airbotix */