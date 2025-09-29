// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file ReaderHandler.cpp
 */

#include "ReaderHandler.hpp"
#include "utils/Logger.hpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

using namespace DDS;

ReaderHandler::ReaderHandler(
        DDS::Topic_var topic,
        DDS::DataReader_var reader,
        DDS::DynamicType_ptr type,
        FastDdsListener* listener,
        const DataTypeConfiguration& data_type_configuration)
    : topic_(topic)
    , reader_(reader)
    , type_(type)
    , listener_(listener)
    , data_type_configuration_(data_type_configuration)
    , data_(nullptr)
    , stop_(false)
{
    create_data_structures_();
}

ReaderHandler::~ReaderHandler()
{
    if (data_)
    {
        // Implement cleanup logic
    }
}

ReaderHandler& ReaderHandler::operator=(ReaderHandler&& other)
{
    if (this != &other)
    {
        topic_ = other.topic_;
        reader_ = other.reader_;
        type_ = other.type_;
        listener_ = other.listener_;
        data_ = other.data_;
        stop_ = other.stop_.load();
        numeric_data_info_ = std::move(other.numeric_data_info_);
        string_data_info_ = std::move(other.string_data_info_);
        data_type_configuration_ = std::move(other.data_type_configuration_);
        other.data_ = nullptr;
    }
    return *this;
}

void ReaderHandler::stop()
{
    stop_ = true;
}

void ReaderHandler::create_data_structures_()
{
    // Placeholder: Implement logic to initialize data structures
}

void ReaderHandler::on_requested_deadline_missed(
        DDS::DataReader_ptr,
        const DDS::RequestedDeadlineMissedStatus&)
{
}

void ReaderHandler::on_requested_incompatible_qos(
        DDS::DataReader_ptr,
        const DDS::RequestedIncompatibleQosStatus&)
{
}

void ReaderHandler::on_sample_rejected(
        DDS::DataReader_ptr,
        const DDS::SampleRejectedStatus&)
{
}

void ReaderHandler::on_liveliness_changed(
        DDS::DataReader_ptr,
        const DDS::LivelinessChangedStatus&)
{
}

void ReaderHandler::on_data_available(DDS::DataReader_ptr)
{
}

void ReaderHandler::on_subscription_matched(
        DDS::DataReader_ptr,
        const DDS::SubscriptionMatchedStatus&)
{
}

void ReaderHandler::on_sample_lost(
        DDS::DataReader_ptr,
        const DDS::SampleLostStatus&)
{
}

std::vector<types::DatumLabel> ReaderHandler::numeric_data_series_names() const
{
    std::vector<types::DatumLabel> names;
    for (const auto& [name, _] : numeric_data_info_)
    {
        names.push_back(name);
    }
    return names;
}

std::vector<types::DatumLabel> ReaderHandler::string_data_series_names() const
{
    std::vector<types::DatumLabel> names;
    for (const auto& [name, _] : string_data_info_)
    {
        names.push_back(name);
    }
    return names;
}

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */