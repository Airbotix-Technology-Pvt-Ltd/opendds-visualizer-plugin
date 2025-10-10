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
 * @file Exception.hpp
 */

#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_UTILS_EXCEPTION_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_UTILS_EXCEPTION_HPP_

#include <stdexcept>
#include <string>

namespace airbotix {
namespace plotjuggler {

/**
 * @brief Base class for all exceptions thrown by the Airbotix PlotJuggler OpenDDS plugin.
 *
 * @note This class extends the std::runtime_error class to inherit functionality to be created with a custom message.
 */
class Exception : public std::runtime_error
{
public:

    using std::runtime_error::runtime_error;
};

class UnsupportedException : public Exception
{
public:

    using Exception::Exception;
};

class InitializationException : public Exception
{
public:

    using Exception::Exception;
};

class InconsistencyException : public Exception
{
public:

    using Exception::Exception;
};

class IncorrectParamException : public Exception
{
public:

    using Exception::Exception;
};

} // namespace plotjuggler
} // namespace airbotix

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_UTILS_EXCEPTION_HPP_