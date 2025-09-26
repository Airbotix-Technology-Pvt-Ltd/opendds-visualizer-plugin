#include "ReaderHandler.hpp"
#include <dds/DCPS/XTypes/DynamicDataImpl.h>
#include <dds/DCPS/XTypes/DynamicDataReaderImpl.h>
#include <iostream>

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

ReaderHandler::ReaderHandler(
        DDS::Topic_var topic,
        DDS::DataReader_var datareader,
        DDS::DynamicType_ptr type,
        FastDdsListener* listener,
        const DataTypeConfiguration& data_type_configuration)
    : listener_(listener)
    , topic_(topic)
    , reader_(datareader)
    , type_(type)
    , data_(nullptr)
    , stop_(false)
    , data_type_configuration_(data_type_configuration)
{
    create_data_structures_();
}

ReaderHandler::~ReaderHandler()
{
    stop();
}

ReaderHandler& ReaderHandler::operator=(ReaderHandler&& other)
{
    listener_ = other.listener_;
    topic_ = other.topic_;
    reader_ = other.reader_;
    type_ = other.type_;
    data_ = other.data_;
    stop_ = other.stop_.load();
    numeric_data_info_ = std::move(other.numeric_data_info_);
    string_data_info_ = std::move(other.string_data_info_);
    data_type_configuration_ = std::move(other.data_type_configuration_);
    return *this;
}

ReaderHandler& ReaderHandler::operator=(const ReaderHandler& other)
{
    listener_ = other.listener_;
    topic_ = other.topic_;
    reader_ = other.reader_;
    type_ = other.type_;
    data_ = other.data_;
    stop_ = other.stop_.load();
    numeric_data_info_ = other.numeric_data_info_;
    string_data_info_ = other.string_data_info_;
    data_type_configuration_ = other.data_type_configuration_;
    return *this;
}

void ReaderHandler::stop()
{
    stop_ = true;
}

void ReaderHandler::on_data_available(DDS::DataReader* reader)
{
    if (stop_) return;

    DDS::DynamicDataReader_var dynamic_reader = DDS::DynamicDataReader::_narrow(reader);
    if (!dynamic_reader) {
        std::cerr << "ReaderHandler: Failed to narrow DataReader to DynamicDataReader" << std::endl;
        return;
    }

    DDS::DynamicDataSeq data_seq;
    DDS::SampleInfoSeq info_seq;
    DDS::ReturnCode_t rc = dynamic_reader->take(
        data_seq,
        info_seq,
        DDS::LENGTH_UNLIMITED,
        DDS::ANY_SAMPLE_STATE,
        DDS::ANY_VIEW_STATE,
        DDS::ANY_INSTANCE_STATE);

    if (rc != DDS::RETCODE_OK) {
        std::cerr << "ReaderHandler: take failed with code " << rc << std::endl;
        return;
    }

    for (CORBA::ULong i = 0; i < data_seq.length(); ++i) {
        if (info_seq[i].valid_data) {
            nlohmann::json json_data;
            rc = utils::serialize_data(data_seq[i], json_data);
            if (rc == DDS::RETCODE_OK) {
                numeric_data_info_.clear();
                string_data_info_.clear();
                utils::get_formatted_data(
                    topic_name(),
                    data_type_configuration_,
                    numeric_data_info_,
                    string_data_info_,
                    json_data);

                // Convert SampleInfo timestamp to seconds
                double timestamp = info_seq[i].source_timestamp.sec +
                                   info_seq[i].source_timestamp.nanosec / 1e9;

                if (listener_) {
                    // Call FastDdsListener methods
                    if (!numeric_data_info_.empty()) {
                        listener_->on_double_data_read(numeric_data_info_, timestamp);
                    }
                    if (!string_data_info_.empty()) {
                        listener_->on_string_data_read(string_data_info_, timestamp);
                    }
                } else {
                    std::cerr << "ReaderHandler: Listener is null" << std::endl;
                }
            } else {
                std::cerr << "ReaderHandler: Serialization failed for sample " << i << std::endl;
            }
        }
    }

    // Return loaned data
    dynamic_reader->return_loan(data_seq, info_seq);
}

std::string ReaderHandler::topic_name() const
{
    return topic_ ? topic_->get_name() : "";
}

std::string ReaderHandler::type_name() const
{
    return type_ ? type_->get_name() : "";
}

std::vector<types::DatumLabel> ReaderHandler::numeric_data_series_names() const
{
    return utils::get_introspection_type_names(numeric_data_info_);
}

std::vector<types::DatumLabel> ReaderHandler::string_data_series_names() const
{
    return utils::get_introspection_type_names(string_data_info_);
}

void ReaderHandler::create_data_structures_(DDS::DynamicData_ptr data)
{
    if (!data && type_) {
        data_ = new OpenDDS::XTypes::DynamicDataImpl(type_);
    } else {
        data_ = data;
    }
    numeric_data_info_.clear();
    string_data_info_.clear();
}

DDS::StatusMask ReaderHandler::default_listener_mask_()
{
    return DDS::DATA_AVAILABLE_STATUS;
}

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */