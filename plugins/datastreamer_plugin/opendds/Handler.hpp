// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Visualizer Plugin.
//
// eProsima Fast DDS Visualizer Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Visualizer Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Visualizer Plugin. If not, see <https://www.gnu.org/licenses/>.

/**
 * @file Handler.hpp
 */

#ifndef _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_HANDLER_HPP_
#define _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_HANDLER_HPP_

#include <memory>
#include <map>      // ← ADD
#include <vector>   // ← ADD
#include <string>   // ← ADD

#include "dds_data.h"
#include "publication_monitor.h"
#include "subscription_monitor.h"
#include "topic_monitor.h"

#include "FastDdsListener.hpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

/**
 * @brief This class handles every Fast DDS entity required.
 *
 * It create, manage and destroy every Fast DDS entity that the process requires to instantiate.
 * The discovery and user data received is transmitted through a FastDdsListener object.
 *
 * FUTURE WORK:
 * Use a specific thread to call callbacks instead of using Fast DDS thread
 */
class Handler
{
public:

    ////////////////////////////////////////////////////
    // CREATION & DESTRUCTION
    ////////////////////////////////////////////////////

    Handler(
            FastDdsListener* listener);

    virtual ~Handler();


    ////////////////////////////////////////////////////
    // INTERACTION METHODS
    ////////////////////////////////////////////////////

    void connect_to_domain(
            const uint32_t domain);

    void disconnect_from_domain();
    
    void start_discovery();

    void create_subscription(
            const std::string& topicName);

    void reset();

    std::vector<types::DatumLabel> numeric_data_series_names() const;

    std::vector<types::DatumLabel> string_data_series_names() const;

protected:

    ////////////////////////////////////////////////////
    // AUXILIAR INTERNAL METHODS
    ////////////////////////////////////////////////////
    
    const char* get_dds_log_prefix() const { return "OpenDDS Handler"; }



    ////////////////////////////////////////////////////
    // INTERNAL VARIABLES
    ////////////////////////////////////////////////////
    DDS::DomainParticipant_var g_participant_;
    DDS::Subscriber_var g_subscriber;

    FastDdsListener* listener_;

    std::unique_ptr<SubscriptionMonitor> m_subscriptionMonitor;
    std::unique_ptr<PublicationMonitor> m_publicationMonitor;

    std::map<std::string, std::shared_ptr<TopicMonitor>> g_topic_monitors;
};

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_HANDLER_HPP_
