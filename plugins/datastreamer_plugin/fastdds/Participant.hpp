// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file Participant.hpp
 */

#ifndef _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_PARTICIPANTS_HPP_
#define _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_PARTICIPANTS_HPP_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <QObject>

#include <dds/DdsDcpsDomainC.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/DCPS_Utils.h>
#include "utils/utils.hpp"
#include "utils/Exception.hpp"
#include "utils/DataTypeConfiguration.hpp"
#include "FastDdsListener.hpp"
#include "ReaderHandler.hpp"
#include "TopicDataBase.hpp"
#include "utils/Logger.cpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

class ReaderHandlerDeleter
{
public:
    ReaderHandlerDeleter(
            DDS::DomainParticipant* participant,
            DDS::Subscriber* subscriber);

    void operator()(ReaderHandler* ptr) const;

protected:
    DDS::DomainParticipant_var participant_;
    DDS::Subscriber_var subscriber_;
};

using ReaderHandlerReference = std::unique_ptr<ReaderHandler, ReaderHandlerDeleter>;

/**
 * @brief This class handles every OpenDDS entity required.
 *
 * It creates, manages, and destroys every OpenDDS entity that the process requires to instantiate.
 * The discovery and user data received is transmitted through a FastDdsListener object.
 */
class Participant
{
public:
    Participant(
            DDS::DomainId_t domain_id,
            std::shared_ptr<TopicDataBase> discovery_database,
            FastDdsListener* listener);

    virtual ~Participant();

    bool register_type_from_xml(const std::string& xml_path);

    void create_subscription(
            const std::string& topic_name,
            const DataTypeConfiguration& data_type_configuration);

    // DDS::DomainParticipantListener methods
    void on_publication_matched(
            DDS::DataWriter_ptr writer,
            const DDS::PublicationMatchedStatus& info);

    std::vector<types::DatumLabel> numeric_data_series_names() const;

    std::vector<types::DatumLabel> string_data_series_names() const;

protected:
    void on_topic_discovery_(
            const std::string& topic_name,
            const std::string& type_name);

    void on_topic_discovery_(
            const std::string& topic_name,
            const std::string& type_name,
            const DataTypeId& type_id);

    DDS::ReturnCode_t get_type_support_from_xml_(
            const std::string& type_name,
            DDS::TypeSupport_var& type_support);

    void check_type_info(
            const std::string& topic_name,
            const std::string& type_name);

    void refresh_types_registered_();

    bool is_type_registered_in_participant_(
            const std::string& type_name);

    static DDS::DomainParticipantQos default_participant_qos_();

    static DDS::SubscriberQos default_subscriber_qos_();

    static DDS::TopicQos default_topic_qos_();

    static DDS::DataReaderQos default_datareader_qos_();

    static DDS::StatusMask default_listener_mask_();

    std::shared_ptr<TopicDataBase> discovery_database_;
    std::shared_ptr<TopicIds> dyn_types_info_;
    FastDdsListener* listener_;

    DDS::DomainParticipantFactory_var factory_;
    DDS::DomainParticipant_var participant_;
    DDS::Subscriber_var subscriber_;
    std::unordered_map<std::string, ReaderHandlerReference> readers_;
};

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_PARTICIPANTS_HPP_