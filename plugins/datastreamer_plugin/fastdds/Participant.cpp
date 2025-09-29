// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file Participant.cpp
 */

#include "Participant.hpp"
#include "utils/utils.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

using namespace DDS;
using namespace OpenDDS::DCPS;

// Placeholder TypeSupport class (to be replaced with actual implementation)
class GenericTypeSupport : public DDS::TypeSupport
{
public:
    GenericTypeSupport(DDS::DynamicType_ptr type, const std::string& type_name)
        : type_(type)
        , type_name_(type_name)
    {
    }

    DDS::ReturnCode_t register_type(
            DDS::DomainParticipant_ptr participant,
            const char* type_name) override
    {
        // Placeholder: Assume type is pre-registered via XML or IDL
        // In a real implementation, use OpenDDS's type registration mechanism
        DDS_DEBUG("GenericTypeSupport", "Registering type %s", type_name);
        return DDS::RETCODE_OK;
    }

    char* get_type_name() override
    {
        return CORBA::string_dup(type_name_.c_str());
    }

    DDS::DynamicType_ptr get_type() override
    {
        return type_;
    }

private:
    DDS::DynamicType_var type_;
    std::string type_name_;
};

////////////////////////////////////////////////////
// READERHANDLER DELETER
////////////////////////////////////////////////////

ReaderHandlerDeleter::ReaderHandlerDeleter(
        DomainParticipant* participant,
        Subscriber* subscriber)
    : participant_(participant)
    , subscriber_(subscriber)
{
}

void ReaderHandlerDeleter::operator()(ReaderHandler* reader) const
{
    reader->stop();
    subscriber_->delete_datareader(reader->get_reader());
    participant_->delete_topic(reader->get_topic());
}

////////////////////////////////////////////////////
// CREATION & DESTRUCTION
////////////////////////////////////////////////////

Participant::Participant(
        DomainId_t domain_id,
        std::shared_ptr<TopicDataBase> discovery_database,
        FastDdsListener* listener)
    : listener_(listener)
    , discovery_database_(discovery_database)
    , dyn_types_info_(std::make_shared<TopicIds>())
{
    std::string dds_root = std::getenv("DDS_ROOT") ? std::getenv("DDS_ROOT") : "/opt/OpenDDS";
    std::string d2_str = dds_root + "/rtps.ini";

    char* d2 = new char[d2_str.length() + 1];
    std::strcpy(d2, d2_str.c_str());

    int dds_argc = 2;
    char d1[] = "-DCPSConfigFile";
    char* dds_argv[] = {d1, d2};

    factory_ = TheParticipantFactoryWithArgs(dds_argc, dds_argv);

    if (!factory_)
    {
        delete[] d2;
        DDS_CRITICAL("Participant", "Failed to get DomainParticipantFactory");
        throw std::runtime_error("Failed to get DomainParticipantFactory");
    }

    participant_ = factory_->create_participant(
        domain_id, default_participant_qos_(), nullptr, default_listener_mask_());

    if (!participant_)
    {
        delete[] d2;
        throw InitializationException("Error creating Domain Participant");
    }

    delete[] d2;

    DDS_DEBUG("Participant", "Participant created in domain %d", domain_id);

    subscriber_ = participant_->create_subscriber(
        default_subscriber_qos_(), nullptr, default_listener_mask_());

    if (!subscriber_)
    {
        factory_->delete_participant(participant_);
        participant_ = nullptr;
        throw InitializationException("Error creating Subscriber");
    }
}

Participant::~Participant()
{
    DDS_DEBUG("Participant", "Destroying Participant");

    if (participant_)
    {
        participant_->delete_contained_entities();
        if (factory_)
        {
            factory_->delete_participant(participant_);
        }
    }
}

////////////////////////////////////////////////////
// INTERACTION METHODS
////////////////////////////////////////////////////

bool Participant::register_type_from_xml(const std::string& xml_path)
{
    DDS_DEBUG("Participant", "Registered types in xml file: %s", xml_path.c_str());
    refresh_types_registered_();
    return true;
}

void Participant::create_subscription(
        const std::string& topic_name,
        const DataTypeConfiguration& data_type_configuration)
{
    DDS_DEBUG("Participant", "Creating subscription for topic: %s", topic_name.c_str());
    if (readers_.find(topic_name) != readers_.end())
    {
        DDS_WARNING("Participant", "Datareader already exists for topic: %s", topic_name.c_str());
        throw InconsistencyException("Trying to create Data Reader again in topic: " + topic_name);
    }

    auto topic_type = discovery_database_->find(topic_name);
    if (topic_type == discovery_database_->end())
    {
        DDS_WARNING("Participant", "Topic %s has not been discovered, so type unknown", topic_name.c_str());
        throw InconsistencyException("Trying to create Data Reader in a non existing topic: " + topic_name);
    }

    DataTypeNameType type_name = discovery_database_->operator[](topic_name).first;
    DDS::TypeSupport_var type_support;
    if (get_type_support_from_xml_(type_name, type_support) != DDS::RETCODE_OK)
    {
        DDS_ERROR("Participant", "Error retrieving type support for %s", type_name.c_str());
        return;
    }

    if (type_support->register_type(participant_, type_name.c_str()) != DDS::RETCODE_OK)
    {
        DDS_ERROR("Participant", "Error registering type %s", type_name.c_str());
        return;
    }

    DDS::Topic_var topic = participant_->create_topic(
        topic_name.c_str(),
        type_name.c_str(),
        default_topic_qos_(),
        nullptr,
        default_listener_mask_());

    if (!topic)
    {
        DDS_ERROR("Participant", "Error creating topic %s", topic_name.c_str());
        return;
    }

    DDS::DataReaderQos dr_qos = default_datareader_qos_();
    DDS::DataReader_var datareader = subscriber_->create_datareader(
        topic,
        dr_qos,
        nullptr,
        default_listener_mask_());

    if (!datareader)
    {
        DDS_ERROR("Participant", "Error creating datareader for topic %s", topic_name.c_str());
        return;
    }

    ReaderHandlerReference new_reader(
        new ReaderHandler(topic, datareader, type_support->get_type(), listener_, data_type_configuration),
        ReaderHandlerDeleter(participant_, subscriber_));

    readers_.insert(std::make_pair(topic_name, std::move(new_reader)));
}

////////////////////////////////////////////////////
// LISTENER METHODS [ PARTICIPANT ]
////////////////////////////////////////////////////

void Participant::on_publication_matched(
        DDS::DataWriter_ptr writer,
        const DDS::PublicationMatchedStatus&)
{
    DDS::SubscriptionBuiltinTopicData data;
    writer->get_matched_subscription_data(data, 0);
    std::string topic_name = data.topic_name.in();
    std::string type_name = data.type_name.in();
    DDS_DEBUG("Participant", "DataWriter discovered in topic: %s [%s]", topic_name.c_str(), type_name.c_str());

    on_topic_discovery_(topic_name, type_name);
}

////////////////////////////////////////////////////
// EXTERNAL EVENT METHODS
////////////////////////////////////////////////////

void Participant::on_topic_discovery_(
        const std::string& topic_name,
        const std::string& type_name)
{
    bool is_already_discovered = false;

    auto it = discovery_database_->find(topic_name);
    if (it != discovery_database_->end())
    {
        is_already_discovered = true;
        DDS_DEBUG("Participant", "Topic %s has already been discovered", topic_name.c_str());
        return;
    }

    if (!is_already_discovered)
    {
        check_type_info(topic_name, type_name);
    }

    if (listener_)
    {
        listener_->on_topic_discovery(topic_name, type_name);
    }
}

void Participant::on_topic_discovery_(
        const std::string& topic_name,
        const std::string& type_name,
        const DataTypeId&)
{
    on_topic_discovery_(topic_name, type_name);
}

////////////////////////////////////////////////////
// RETRIEVE INFORMATION METHODS
////////////////////////////////////////////////////

std::vector<types::DatumLabel> Participant::numeric_data_series_names() const
{
    std::vector<types::DatumLabel> names;
    for (const auto& reader : readers_)
    {
        for (const auto& series : reader.second->numeric_data_series_names())
        {
            names.push_back(series);
        }
    }
    return names;
}

std::vector<types::DatumLabel> Participant::string_data_series_names() const
{
    std::vector<types::DatumLabel> names;
    for (const auto& reader : readers_)
    {
        for (const auto& series : reader.second->string_data_series_names())
        {
            names.push_back(series);
        }
    }
    return names;
}

////////////////////////////////////////////////////
// AUXILIAR METHODS
////////////////////////////////////////////////////

DDS::ReturnCode_t Participant::get_type_support_from_xml_(
        const std::string& type_name,
        DDS::TypeSupport_var& type_support)
{
    // Placeholder: Assume types are pre-registered via XML
    DDS::DynamicType_var type;
    type_support = new GenericTypeSupport(type, type_name);
    return DDS::RETCODE_OK;
}

void Participant::check_type_info(
        const std::string& topic_name,
        const std::string& type_name)
{
    DDS::TypeSupport_var type_support;
    if (get_type_support_from_xml_(type_name, type_support) != DDS::RETCODE_OK)
    {
        DDS_WARNING("Participant", "Type information of %s is currently not available", type_name.c_str());
        discovery_database_->operator[](topic_name) = {type_name, false};
        return;
    }

    // Check if type is already registered
    DDS::TypeSupport_var existing_support;
    // Placeholder: Assume type is registered
    if (!existing_support)
    {
        DDS_DEBUG("Participant", "Type info available. Registering type %s in participant", type_name.c_str());
        discovery_database_->operator[](topic_name) = {type_name, true};
        type_support->register_type(participant_, type_name.c_str());
    }
}

void Participant::refresh_types_registered_()
{
    for (auto const& [topic_name, topic_data_type_info] : *discovery_database_)
    {
        if (std::get<TypeInfoAvailable>(topic_data_type_info))
        {
            continue;
        }
        else
        {
            check_type_info(topic_name, std::get<DataTypeNameType>(topic_data_type_info));
        }

        if (listener_)
        {
            listener_->on_topic_discovery(topic_name, std::get<DataTypeNameType>(topic_data_type_info));
        }
    }
}

bool Participant::is_type_registered_in_participant_(
        const std::string& type_name)
{
    // Placeholder: Implement actual type check if needed
    return true;
}

////////////////////////////////////////////////////
// AUXILIAR STATIC METHODS
////////////////////////////////////////////////////

DDS::DomainParticipantQos Participant::default_participant_qos_()
{
    DDS::DomainParticipantQos qos = TheServiceParticipant->initial_DomainParticipantQos();
    qos.entity_factory.autoenable_created_entities = true;
    return qos;
}

DDS::SubscriberQos Participant::default_subscriber_qos_()
{
    DDS::SubscriberQos qos = TheServiceParticipant->initial_SubscriberQos();
    qos.entity_factory.autoenable_created_entities = true;
    return qos;
}

DDS::DataReaderQos Participant::default_datareader_qos_()
{
    DDS::DataReaderQos qos = TheServiceParticipant->initial_DataReaderQos();
    return qos;
}

DDS::TopicQos Participant::default_topic_qos_()
{
    DDS::TopicQos qos = TheServiceParticipant->initial_TopicQos();
    return qos;
}

DDS::StatusMask Participant::default_listener_mask_()
{
    return OpenDDS::DCPS::DEFAULT_STATUS_MASK;
}

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */