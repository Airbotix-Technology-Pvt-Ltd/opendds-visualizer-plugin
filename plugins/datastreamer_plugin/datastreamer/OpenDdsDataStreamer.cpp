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
 * @file OpenDdsDataStreamer.cpp
 */

#include "OpenDdsDataStreamer.hpp"
#include "ui/topic_selection_dialog/dialogselecttopics.h"
#include "utils/utils.hpp"
#include "utils/Exception.hpp"

namespace airbotix {
namespace plotjuggler {
namespace datastreamer {

OpenDdsDataStreamer::OpenDdsDataStreamer()
    : running_(false)
    , configuration_(QString(CONFIGURATION_SETTINGS_PREFIX_))
    , opendds_handler_(this)
    , select_topics_dialog_(
        configuration_, this)
{
    DEBUG("Create OpenDdsDataStreamer");
}

OpenDdsDataStreamer::~OpenDdsDataStreamer()
{
    DEBUG("Destroy OpenDdsDataStreamer");
    shutdown();
}

bool OpenDdsDataStreamer::start(
        QStringList*)
{
    DEBUG("OpenDdsDataStreamer::start");

    // Check if it is already running
    if (running_)
    {
        return true;
    }

    // Creating a default DomainParticipant in domain by default (configuration_)
    this->connect_to_domain_(configuration_.domain_id);

    // Reset dialogselecttopic to current configuration
    // NOTE: If this is done before connecting to domain, xml files will not be added
    select_topics_dialog_.reset_to_configuration_(configuration_);

    // Execute Dialog
    int dialog_result = select_topics_dialog_.exec();

    // Check if Accept has been pressed
    if (dialog_result != QDialog::Accepted)
    {
        DEBUG("Dialog closed cancelled, exiting");
        return false;
    }

    // Get configuration from dialog
    configuration_ = select_topics_dialog_.get_configuration();
    // Store as default configuration
    configuration_.save_default_settings(CONFIGURATION_SETTINGS_PREFIX_);

    // Topics selected
    const auto& topics = configuration_.topics_selected;  // Decorator variable to avoid calling internal member

    if (topics.empty())
    {
        DEBUG("No topics selected, exiting");
        throw InitializationException("No topics selected.");
    }

    for (const auto& topic : topics)
    {
        // Create a subscription
        opendds_handler_.create_subscription(
            utils::QString_to_string(topic), configuration_.data_type_configuration);
    }

    // Get all series from topics and create them
    dataMap().clear();
    create_series_();

    running_ = true;
    return true;
}

void OpenDdsDataStreamer::shutdown()
{
    DEBUG("Bye World");

    // If it is running, stop it
    if (running_)
    {
        running_ = false;

        // Reset OpenDDS so DDS entities are destroyed
        opendds_handler_.reset();
        select_topics_dialog_.reset();
    }
}

bool OpenDdsDataStreamer::isRunning() const
{
    return running_;
}

const char* OpenDdsDataStreamer::name() const
{
    return PLUGIN_NAME_;
}

bool OpenDdsDataStreamer::xmlSaveState(
        QDomDocument& doc,
        QDomElement& plugin_elem) const
{
    return configuration_.xmlSaveState(doc, plugin_elem);
}

bool OpenDdsDataStreamer::xmlLoadState(
        const QDomElement& parent_element)
{
    return configuration_.xmlLoadState(parent_element);
}

////////////////////////////////////////////////////
// OPENDDS LISTENER METHODS
////////////////////////////////////////////////////

void OpenDdsDataStreamer::on_data_available()
{
    DEBUG("OpenDdsDataStreamer on_data_available");

    // Locking DataStream
    std::lock_guard<std::mutex> lock(mutex());

    // Create series from new received sample
    create_series_();
}

void OpenDdsDataStreamer::on_double_data_read(
        const std::vector<std::pair<std::string, double>>& data_per_topic_value,
        double timestamp)
{
    DEBUG("OpenDdsDataStreamer on_double_data_read");

    // Locking DataStream
    std::lock_guard<std::mutex> lock(mutex());

    for (const auto& data : data_per_topic_value)
    {
        DEBUG("Adding to numeric series " << data.first << " value " << data.second << " with timestamp " << timestamp);
        if (dataMap().numeric.find(data.first) == dataMap().numeric.end())
        {
            throw InconsistencyException("Series " + data.first + " not found.");
        }
        // Get data map
        auto& series = dataMap().numeric.find(data.first)->second;

        // Add data to series
        series.pushBack( { timestamp, data.second});
        DEBUG("...Data added to series");
    }

    emit dataReceived();
}

void OpenDdsDataStreamer::on_string_data_read(
        const std::vector<std::pair<std::string, std::string>>& data_per_topic_value,
        double timestamp    )
{
    DEBUG("OpenDdsDataStreamer on_string_data_read");

    // Locking DataStream
    std::lock_guard<std::mutex> lock(mutex());

    for (const auto& data : data_per_topic_value)
    {
        DEBUG("Adding to string series " << data.first << " value " << data.second << " with timestamp " << timestamp);

        // Get data map
        auto& series = dataMap().strings.find(data.first)->second;
        // Add data to series
        series.pushBack( { timestamp, data.second});
    }

    emit dataReceived();
}

void OpenDdsDataStreamer::on_topic_discovery(
        const std::string& topic_name,
        const std::string& type_name)
{
    DEBUG("OpenDdsDataStreamer topic_discovery_signal " << topic_name);

    // Emit signal to UI so it is handled from Qt thread
    emit select_topics_dialog_.topic_discovery_signal(
        utils::string_to_QString(topic_name),
        utils::string_to_QString(type_name),
        true);
}

////////////////////////////////////////////////////
// UI LISTENER METHODS
////////////////////////////////////////////////////

void OpenDdsDataStreamer::on_domain_connection(
        unsigned int domain_id)
{
    DEBUG("OpenDdsDataStreamer on_domain_connection " << domain_id);
    connect_to_domain_(domain_id);
}

////////////////////////////////////////////////////
// AUXILIAR METHODS
////////////////////////////////////////////////////

void OpenDdsDataStreamer::connect_to_domain_(
        unsigned int domain_id)
{
    DEBUG("OpenDdsDataStreamer connect_to_domain_ " << domain_id);

    // Reset view and handler
    select_topics_dialog_.reset();
    opendds_handler_.reset();

    // Connect to domain
    opendds_handler_.connect_to_domain(domain_id);
    select_topics_dialog_.connect_to_domain(domain_id);
}

void OpenDdsDataStreamer::create_series_()
{
    // Get all series from topics and create them
    // NUMERIC
    std::vector<types::DatumLabel> numeric_series = opendds_handler_.numeric_data_series_names();
    for (const auto& series : numeric_series)
    {
        // Create a series
        DEBUG("Creating numeric series: " << series);
        dataMap().addNumeric(series);
    }

    // STRING
    std::vector<types::DatumLabel> string_series = opendds_handler_.string_data_series_names();
    for (const auto& series : string_series)
    {
        // Create a series
        DEBUG("Creating string series: " << series);
        dataMap().addStringSeries(series);
    }
}

} /* namespace datastreamer */
} /* namespace plotjuggler */
} /* namespace airbotix */