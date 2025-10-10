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
 * @file Handler.hpp
 */

#include "Handler.hpp"

namespace airbotix {
namespace plotjuggler {
namespace opendds {

////////////////////////////////////////////////////
// CREATION & DESTRUCTION
////////////////////////////////////////////////////

Handler::Handler(
        OpenDdsListener* listener)
    : listener_(listener)
{
    // TOOD remove
    // Activate opendds warning logger
    airbotix::dds::init_logging_from_env();
    // Do nothing
}

Handler::~Handler()
{
    // Destroy internal participant explicitly before this object is destroyed
    reset();
}

////////////////////////////////////////////////////
// INTERACTION METHODS
////////////////////////////////////////////////////

void Handler::connect_to_domain(
        const uint32_t domain)
{
    // Reset in case a Handler exist
    reset();

    // Create participant
    DDS_INFO("Main", "Starting DDS Data Recorder");

    try {
        // Instantiate DDSManager
        CommonData::m_ddsManager = std::make_unique<DDSManager>();

        // Join domain with callbacks for participant add/remove events
        CommonData::m_ddsManager->joinDomain(domain);

        // Retrieve the DomainParticipant
        g_participant_ = CommonData::m_ddsManager->getDomainParticipant();

        if (!g_participant_) {
            DDS_ERROR("Main", "Failed to get DomainParticipant");
            throw std::runtime_error("Failed to get DomainParticipant");
        }

        start_discovery();
    } catch (const std::exception& e) {
        DDS_ERROR("Main", "Exception during joinDomain: %s", e.what());
        throw;  // Re-throw the exception
    }
}

void Handler::start_discovery(){
    if (!g_participant_) {
        DDS_ERROR(get_dds_log_prefix(), "Failed to get DomainParticipant", "");
        throw std::runtime_error("No DomainParticipant");
    }

    DDS::SubscriberQos sq;
    g_participant_->get_default_subscriber_qos(sq);
    g_subscriber = g_participant_->create_subscriber(sq, nullptr, OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (!g_subscriber) {
        DDS_ERROR(get_dds_log_prefix(), "Failed to create subscriber", "");
        throw std::runtime_error("Failed to create subscriber");
    }

    m_subscriptionMonitor = std::make_unique<SubscriptionMonitor>(g_participant_);
    m_publicationMonitor = std::make_unique<PublicationMonitor>(g_participant_);

    m_publicationMonitor->set_new_topic_callback(
        [this](const std::string& topicName, const std::string& type_name) {
            // Call listener callback to notify new topic
            if (listener_)
            {
                listener_->on_topic_discovery(topicName, type_name);
            }
            DDS_DEBUG(get_dds_log_prefix(), "Discovered publication on topic: %s", topicName.c_str());
        });
}


void Handler::create_subscription(
        const std::string& topicName, const DataTypeConfiguration& data_type_configuration)
{
    DDS_DEBUG(get_dds_log_prefix(), "Creating TopicMonitor and thread for topic: %s", topicName.c_str());

    if (g_topic_monitors.find(topicName) != g_topic_monitors.end()) {
        DDS_DEBUG(get_dds_log_prefix(), "TopicMonitor already exists for: %s", topicName.c_str());
        return;
    }

    try {
        auto monitor = std::make_shared<TopicMonitor>(topicName, g_participant_, listener_, data_type_configuration);
        g_topic_monitors[topicName] = monitor;

        DDS_INFO(get_dds_log_prefix(), "Created TopicMonitor and spawned thread for topic: %s", topicName.c_str());
    } catch (const std::exception& e) {
        DDS_ERROR(get_dds_log_prefix(), "Failed to create TopicMonitor for %s: %s", topicName.c_str(), e.what());
    }

}

void Handler::reset()
{
    CommonData::cleanup();
}

std::vector<types::DatumLabel> Handler::numeric_data_series_names() const
{
    std::vector<types::DatumLabel> names;

    for (const auto& reader : g_topic_monitors)
    {
        for (const auto& series : reader.second->numeric_data_series_names())
        {
            names.push_back(series);
        }
    }

    return names;
}

std::vector<types::DatumLabel> Handler::string_data_series_names() const
{
    std::vector<types::DatumLabel> names;

    for (const auto& reader : g_topic_monitors)
    {
        for (const auto& series : reader.second->string_data_series_names())
        {
            names.push_back(series);
        }
    }

    return names;
}

} /* namespace opendds */
} /* namespace plotjuggler */
} /* namespace airbotix */