// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

#ifndef READER_HANDLER_HPP
#define READER_HANDLER_HPP

#include <atomic>
#include <map>
#include <string>
#include <vector>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/XTypes/DynamicDataImpl.h>
#include "FastDdsListener.hpp"
#include "utils/DataTypeConfiguration.hpp"
#include "utils/types.hpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

class ReaderHandler : public DDS::DataReaderListener
{
public:
    ReaderHandler(
            DDS::Topic_var topic,
            DDS::DataReader_var reader,
            DDS::DynamicType_ptr type,
            FastDdsListener* listener,
            const DataTypeConfiguration& config);

    ~ReaderHandler();

    ReaderHandler& operator=(ReaderHandler&& other);
    ReaderHandler& operator=(const ReaderHandler& other) = delete;

    void stop();

    DDS::DataReader_var get_reader() const { return reader_; }
    DDS::Topic_var get_topic() const { return topic_; }

    void on_requested_deadline_missed(
            DDS::DataReader_ptr,
            const DDS::RequestedDeadlineMissedStatus&) override;

    void on_requested_incompatible_qos(
            DDS::DataReader_ptr,
            const DDS::RequestedIncompatibleQosStatus&) override;

    void on_sample_rejected(
            DDS::DataReader_ptr,
            const DDS::SampleRejectedStatus&) override;

    void on_liveliness_changed(
            DDS::DataReader_ptr,
            const DDS::LivelinessChangedStatus&) override;

    void on_data_available(DDS::DataReader_ptr) override;

    void on_subscription_matched(
            DDS::DataReader_ptr,
            const DDS::SubscriptionMatchedStatus&) override;

    void on_sample_lost(
            DDS::DataReader_ptr,
            const DDS::SampleLostStatus&) override;

    std::vector<types::DatumLabel> numeric_data_series_names() const;
    std::vector<types::DatumLabel> string_data_series_names() const;

protected:
    void create_data_structures_();

    DDS::Topic_var topic_;
    DDS::DataReader_var reader_;
    DDS::DynamicType_ptr type_;
    FastDdsListener* listener_;
    DataTypeConfiguration data_type_configuration_;
    void* data_;
    std::atomic<bool> stop_;
    std::map<std::string, std::string> numeric_data_info_;
    std::map<std::string, std::string> string_data_info_;
};

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // READER_HANDLER_HPP