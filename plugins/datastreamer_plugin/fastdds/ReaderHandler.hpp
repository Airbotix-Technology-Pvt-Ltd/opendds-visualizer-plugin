// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file ReaderHandler.hpp
 */

#ifndef _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_READERHANDLER_HPP_
#define _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_READERHANDLER_HPP_

#include <dds/DdsDcpsDomainC.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/XTypes/DynamicDataFactory.h>
#include <dds/DCPS/XTypes/DynamicTypeSupport.h>
#include "utils/utils.hpp"
#include "utils/dynamic_types_utils.hpp"
#include "FastDdsListener.hpp"

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
            const DataTypeConfiguration& data_type_configuration);

    virtual ~ReaderHandler();

    ReaderHandler& operator=(ReaderHandler&& other);

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

    void on_data_available(
            DDS::DataReader_ptr reader) override;

    void on_subscription_matched(
            DDS::DataReader_ptr,
            const DDS::SubscriptionMatchedStatus&) override;

    void on_sample_lost(
            DDS::DataReader_ptr,
            const DDS::SampleLostStatus&) override;

    std::vector<types::DatumLabel> numeric_data_series_names() const;
    std::vector<types::DatumLabel> string_data_series_names() const;

protected:
    void create_data_structures_(DDS::DynamicData_ptr data);

    static DDS::StatusMask default_listener_mask_();

    DDS::Topic_var topic_;
    DDS::DataReader_var reader_;
    DDS::DynamicType_ptr type_;
    FastDdsListener* listener_;
    DataTypeConfiguration data_type_configuration_;
    DDS::DynamicData_ptr data_;
    std::atomic<bool> stop_;
    std::vector<types::NumericDatum> numeric_data_info_;
    std::vector<types::TextDatum> string_data_info_;
};

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_READERHANDLER_HPP_