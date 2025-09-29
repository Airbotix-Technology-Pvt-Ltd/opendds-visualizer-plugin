// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file ReaderHandler.cpp
 */

#include "ReaderHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "utils/dynamic_types_utils.hpp"
#include <dds/DCPS/XTypes/DynamicDataFactory.h>
#include <dds/DCPS/XTypes/DynamicTypeSupport.h>
#include <dds/DCPS/XTypes/DynamicDataReaderImpl.h>

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
    DDS_DEBUG("ReaderHandler", "Creating ReaderHandler for topic: %s", topic->get_name());
    // Create data to avoid reallocation
    data_ = DDS::DynamicDataFactory::get_instance()->create_data(type_);
    // Set this object as the reader's listener
    reader_->set_listener(this, default_listener_mask_());
}

ReaderHandler::~ReaderHandler()
{
    DDS_DEBUG("ReaderHandler", "Destroying ReaderHandler for topic: %s", topic_->get_name());
    stop();
    if (data_)
    {
        DDS::DynamicDataFactory::get_instance()->delete_data(data_);
        data_ = nullptr;
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
    DDS_DEBUG("ReaderHandler", "Stopping ReaderHandler for topic: %s", topic_->get_name());
    stop_ = true;
    reader_->set_listener(nullptr, DDS::StatusMask());
}

void ReaderHandler::create_data_structures_(DDS::DynamicData_ptr data)
{
    DDS_DEBUG("ReaderHandler", "Creating data structures for topic: %s", topic_->get_name());
    if (!data)
    {
        DDS_ERROR("ReaderHandler", "Data is null for topic: %s", topic_->get_name());
        return;
    }

    nlohmann::json serialized_data;
    if (RETCODE_OK != utils::serialize_data(data, serialized_data))
    {
        DDS_ERROR("ReaderHandler", "Error serializing data for topic: %s", topic_->get_name());
        return;
    }

    numeric_data_info_.clear();
    string_data_info_.clear();
    utils::get_formatted_data(
        topic_->get_name(),
        data_type_configuration_,
        numeric_data_info_,
        string_data_info_,
        serialized_data);

    DDS_DEBUG("ReaderHandler", "Completed type introspection for topic: %s", topic_->get_name());
    for (const auto& info : numeric_data_info_)
    {
        DDS_DEBUG("ReaderHandler", "Numeric: %s", std::get<0>(info).c_str());
    }
    for (const auto& info : string_data_info_)
    {
        DDS_DEBUG("ReaderHandler", "String: %s", std::get<0>(info).c_str());
    }
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

void ReaderHandler::on_data_available(DDS::DataReader_ptr reader)
{
    const char* topic_name = topic_->get_name();
    DDS_DEBUG("ReaderHandler", "Data available for topic: %s", topic_name);

    if (!reader)
    {
        DDS_ERROR("ReaderHandler", "Reader pointer is null! Topic: %s", topic_name);
        return;
    }

    try
    {
        // Check subscription status
        DDS::SubscriptionMatchedStatus sub_status;
        if (reader->get_subscription_matched_status(sub_status) == DDS::RETCODE_OK)
        {
            DDS_INFO("ReaderHandler", "Subscription status - Current matches: %d, Total matches: %d for topic: %s",
                     sub_status.current_count, sub_status.total_count, topic_name);
        }

        // Check sample rejected status
        DDS::SampleRejectedStatus rejected_status;
        if (reader->get_sample_rejected_status(rejected_status) == DDS::RETCODE_OK)
        {
            if (rejected_status.total_count > 0)
            {
                DDS_ERROR("ReaderHandler", "Samples rejected - Total: %d, Last reason: %d for topic: %s",
                          rejected_status.total_count, rejected_status.last_reason, topic_name);
            }
        }

        // Check sample lost status
        DDS::SampleLostStatus lost_status;
        if (reader->get_sample_lost_status(lost_status) == DDS::RETCODE_OK)
        {
            if (lost_status.total_count > 0)
            {
                DDS_WARNING("ReaderHandler", "Samples lost - Total: %d for topic: %s",
                            lost_status.total_count, topic_name);
            }
        }

        // Narrow to DynamicDataDataReader
        DDS::DynamicDataReader_var dynamic_reader = DDS::DynamicDataReader::_narrow(reader);
        if (!dynamic_reader)
        {
            DDS_ERROR("ReaderHandler", "Failed to narrow DataReader to DynamicDataDataReader for topic: %s", topic_name);
            return;
        }

        // Read data
        DDS::DynamicDataSeq data_seq;
        DDS::SampleInfoSeq info_seq;
        DDS::ReturnCode_t read_ret = dynamic_reader->read(
            data_seq,
            info_seq,
            DDS::LENGTH_UNLIMITED,
            DDS::ANY_SAMPLE_STATE,
            DDS::ANY_VIEW_STATE,
            DDS::ALIVE_INSTANCE_STATE);

        if (read_ret != DDS::RETCODE_OK)
        {
            DDS_ERROR("ReaderHandler", "Failed to read data for topic: %s, error: %d", topic_name, read_ret);
            return;
        }

        for (CORBA::ULong i = 0; i < data_seq.length(); ++i)
        {
            if (info_seq[i].valid_data)
            {
                // Get timestamp
                double timestamp = utils::get_timestamp_seconds_numeric_value(info_seq[i].source_timestamp);

                // Process data
                create_data_structures_(data_seq[i]);

                // Notify listener
                listener_->on_data_available();

                // Send numeric data
                if (!numeric_data_info_.empty())
                {
                    listener_->on_double_data_read(numeric_data_info_, timestamp);
                }

                // Send string data
                if (!string_data_info_.empty())
                {
                    listener_->on_string_data_read(string_data_info_, timestamp);
                }
            }
        }

        dynamic_reader->return_loan(data_seq, info_seq);
    }
    catch (const CORBA::Exception& e)
    {
        DDS_ERROR("ReaderHandler", "Error processing sample for topic %s: %s", topic_name, e._info().c_str());
    }
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
    for (const auto& info : numeric_data_info_)
    {
        names.push_back(std::get<0>(info));
    }
    return names;
}

std::vector<types::DatumLabel> ReaderHandler::string_data_series_names() const
{
    std::vector<types::DatumLabel> names;
    for (const auto& info : string_data_info_)
    {
        names.push_back(std::get<0>(info));
    }
    return names;
}

DDS::StatusMask ReaderHandler::default_listener_mask_()
{
    DDS::StatusMask mask = DDS::StatusMask();
    mask |= DDS::DATA_AVAILABLE_STATUS;
    return mask;
}

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */