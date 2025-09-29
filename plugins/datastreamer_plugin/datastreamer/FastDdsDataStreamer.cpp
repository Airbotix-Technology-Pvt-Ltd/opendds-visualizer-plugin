// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file FastDdsDataStreamer.cpp
 */

#include "FastDdsDataStreamer.hpp"
#include "ui/topic_selection_dialog/dialogselecttopics.h"
#include "utils/utils.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"

namespace eprosima {
namespace plotjuggler {
namespace datastreamer {

FastDdsDataStreamer::FastDdsDataStreamer()
    : running_(false)
    , configuration_(QString(CONFIGURATION_SETTINGS_PREFIX_))
    , fastdds_handler_(this)
    , select_topics_dialog_(
        configuration_,
        fastdds_handler_.get_topic_data_base(),
        this)
{
    DDS_DEBUG("FastDdsDataStreamer", "Create FastDdsDataStreamer");
}

FastDdsDataStreamer::~FastDdsDataStreamer()
{
    DDS_DEBUG("FastDdsDataStreamer", "Destroy FastDdsDataStreamer");
    shutdown();
}

bool FastDdsDataStreamer::start(
        QStringList*)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer::start");

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
        DDS_DEBUG("FastDdsDataStreamer", "Dialog closed cancelled, exiting");
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
        DDS_DEBUG("FastDdsDataStreamer", "No topics selected, exiting");
        throw InitializationException("No topics selected.");
    }

    for (const auto& topic : topics)
    {
        // Create a subscription
        fastdds_handler_.create_subscription(
            utils::QString_to_string(topic),
            configuration_.data_type_configuration);
    }

    // Get all series from topics and create them
    dataMap().clear();
    create_series_();

    running_ = true;
    return true;
}

void FastDdsDataStreamer::shutdown()
{
    DDS_DEBUG("FastDdsDataStreamer", "Bye World");

    // If it is running, stop it
    if (running_)
    {
        running_ = false;

        // Reset FastDDS so DDS entities are destroyed
        fastdds_handler_.reset();
        select_topics_dialog_.reset();
    }
}

bool FastDdsDataStreamer::isRunning() const
{
    return running_;
}

const char* FastDdsDataStreamer::name() const
{
    return PLUGIN_NAME_;
}

bool FastDdsDataStreamer::xmlSaveState(
        QDomDocument& doc,
        QDomElement& plugin_elem) const
{
    return configuration_.xmlSaveState(doc, plugin_elem);
}

bool FastDdsDataStreamer::xmlLoadState(
        const QDomElement& parent_element)
{
    return configuration_.xmlLoadState(parent_element);
}

////////////////////////////////////////////////////
// FASTDDS LISTENER METHODS
////////////////////////////////////////////////////

void FastDdsDataStreamer::on_data_available()
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer on_data_available");

    // Locking DataStream
    std::lock_guard<std::mutex> lock(mutex());

    // Create series from new received sample
    create_series_();
}

void FastDdsDataStreamer::on_double_data_read(
        const std::vector<std::pair<std::string, double>>& data_per_topic_value,
        double timestamp)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer on_double_data_read");

    // Locking DataStream
    std::lock_guard<std::mutex> lock(mutex());

    for (const auto& data : data_per_topic_value)
    {
        DDS_DEBUG("FastDdsDataStreamer", "Adding to numeric series %s value %f with timestamp %f",
                  data.first.c_str(), data.second, timestamp);
        if (dataMap().numeric.find(data.first) == dataMap().numeric.end())
        {
            throw InconsistencyException("Series " + data.first + " not found.");
        }
        // Get data map
        auto& series = dataMap().numeric.find(data.first)->second;

        // Add data to series
        series.pushBack({timestamp, data.second});
        DDS_DEBUG("FastDdsDataStreamer", "Data added to series");
    }

    emit dataReceived();
}

void FastDdsDataStreamer::on_string_data_read(
        const std::vector<std::pair<std::string, std::string>>& data_per_topic_value,
        double timestamp)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer on_string_data_read");

    // Locking DataStream
    std::lock_guard<std::mutex> lock(mutex());

    for (const auto& data : data_per_topic_value)
    {
        DDS_DEBUG("FastDdsDataStreamer", "Adding to string series %s value %s with timestamp %f",
                  data.first.c_str(), data.second.c_str(), timestamp);

        // Get data map
        auto& series = dataMap().strings.find(data.first)->second;
        // Add data to series
        series.pushBack({timestamp, data.second});
    }

    emit dataReceived();
}

void FastDdsDataStreamer::on_topic_discovery(
        const std::string& topic_name,
        const std::string& type_name)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer topic_discovery_signal %s",
              topic_name.c_str());
    bool type_info_available = fastdds_handler_.get_topic_data_base()->operator[](topic_name).second;

    // Emit signal to UI so it is handled from Qt thread
    emit select_topics_dialog_.topic_discovery_signal(
        utils::string_to_QString(topic_name),
        utils::string_to_QString(type_name),
        type_info_available);
}

////////////////////////////////////////////////////
// UI LISTENER METHODS
////////////////////////////////////////////////////

void FastDdsDataStreamer::on_xml_datatype_file_added(
        const std::string& file_path)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer on_xml_datatype_file_added %s",
              file_path.c_str());
    fastdds_handler_.register_type_from_xml(file_path);
}

void FastDdsDataStreamer::on_domain_connection(
        unsigned int domain_id)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer on_domain_connection %u",
              domain_id);
    connect_to_domain_(domain_id);
}

////////////////////////////////////////////////////
// AUXILIAR METHODS
////////////////////////////////////////////////////

void FastDdsDataStreamer::connect_to_domain_(
        unsigned int domain_id)
{
    DDS_DEBUG("FastDdsDataStreamer", "FastDdsDataStreamer connect_to_domain_ %u",
              domain_id);

    // Reset view and handler
    select_topics_dialog_.reset();
    fastdds_handler_.reset();

    // Connect to domain
    fastdds_handler_.connect_to_domain(domain_id);
    select_topics_dialog_.connect_to_domain(domain_id);
}

void FastDdsDataStreamer::create_series_()
{
    // Get all series from topics and create them
    // NUMERIC
    std::vector<types::DatumLabel> numeric_series = fastdds_handler_.numeric_data_series_names();
    for (const auto& series : numeric_series)
    {
        // Create a series
        DDS_DEBUG("FastDdsDataStreamer", "Creating numeric series: %s", series.c_str());
        dataMap().addNumeric(series);
    }

    // STRING
    std::vector<types::DatumLabel> string_series = fastdds_handler_.string_data_series_names();
    for (const auto& series : string_series)
    {
        // Create a series
        DDS_DEBUG("FastDdsDataStreamer", "Creating string series: %s", series.c_str());
        dataMap().addStringSeries(series);
    }
}

} /* namespace datastreamer */
} /* namespace plotjuggler */
} /* namespace eprosima */