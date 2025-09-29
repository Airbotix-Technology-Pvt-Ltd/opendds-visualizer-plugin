// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file utils.cpp
 */

#include <filesystem>
#include <cstdio> // for snprintf
#include <iomanip>
#include <string>

#include "utils.hpp"
#include "Logger.hpp"
#include <dds/DCPS/TimeTypes.h> // for DDS::Time_t

namespace eprosima {
namespace plotjuggler {
namespace utils {

std::string get_timestamp_string(const DDS::Time_t& timestamp)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld.%09u", timestamp.sec, timestamp.nanosec);
    return std::string(buffer);
}

double get_timestamp_seconds_numeric_value(
        const DDS::Time_t& timestamp)
{
    return (timestamp.sec + (timestamp.nanosec * 1e-9));
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
    return QString::fromUtf8(str.data(), str.size());
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
        std::regex(".*\\." + file_extension + "$"),
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
} /* namespace eprosima */