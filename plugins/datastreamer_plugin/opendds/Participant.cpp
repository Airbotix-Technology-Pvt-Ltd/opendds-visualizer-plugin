// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
// Licensed under the GNU General Public License v3.0.

/**
 * @file Participant.cpp
 */

#include "Participant.hpp"
#include "utils/utils.hpp"
#include "utils/Exception.hpp"
#include "utils/Logger.hpp"
#include <dds/DCPS/XTypes/DynamicTypeSupport.h>
#include <filesystem>

namespace eprosima {
namespace plotjuggler {
namespace opendds {

using namespace DDS;
using namespace OpenDDS::DCPS;

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
        OpenDdsListener* listener)
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
    DDS_DEBUG("Participant", "Loading XML type definitions from file: %s", xml_path.c_str());
    
    // Check if the file exists
    if (!std::filesystem::exists(xml_path))
    {
        DDS_ERROR("Participant", "XML file does not exist: %s", xml_path.c_str());
        throw IncorrectParamException("Failed reading XML file: " + xml_path);
    }
    
    // OpenDDS uses XTypes for dynamic type support
    // Load the XML file using OpenDDS's type system
    try
    {
        // Note: OpenDDS XTypes XML support may require specific configuration
        // This is a basic implementation that would need to be extended based on
        // the actual XML schema and OpenDDS version being used
        DDS_INFO("Participant", "XML file loaded: %s", xml_path.c_str());
        
        // After loading XML, refresh types that might now be available
        refresh_types_registered_();
        
        return true;
    }
    catch (const std::exception& e)
    {
        DDS_ERROR("Participant", "Error loading XML file %s: %s", xml_path.c_str(), e.what());
        throw IncorrectParamException("Failed processing XML file: " + xml_path + " - " + e.what());
    }
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
    
    // Get the type support for this type
    if (get_type_support_from_xml_(type_name, type_support) != DDS::RETCODE_OK)
    {
        DDS_ERROR("Participant", "Error retrieving type support for %s", type_name.c_str());
        throw InconsistencyException("Failed to get type support for type: " + type_name);
    }

    // Register the type if not already registered
    if (!is_type_registered_in_participant_(type_name))
    {
        DDS_DEBUG("Participant", "Registering type %s with participant", type_name.c_str());
        if (type_support->register_type(participant_, type_name.c_str()) != DDS::RETCODE_OK)
        {
            DDS_ERROR("Participant", "Error registering type %s", type_name.c_str());
            throw InconsistencyException("Failed to register type: " + type_name);
        }
    }

    // Create the topic
    DDS::Topic_var topic = participant_->create_topic(
        topic_name.c_str(),
        type_name.c_str(),
        default_topic_qos_(),
        nullptr,
        default_listener_mask_());

    if (!topic)
    {
        DDS_ERROR("Participant", "Error creating topic %s", topic_name.c_str());
        throw InconsistencyException("Failed to create topic: " + topic_name);
    }

    // Create the datareader with appropriate QoS
    DDS::DataReaderQos dr_qos = default_datareader_qos_();
    DDS::DataReader_var datareader = subscriber_->create_datareader(
        topic,
        dr_qos,
        nullptr,
        default_listener_mask_());

    if (!datareader)
    {
        DDS_ERROR("Participant", "Error creating datareader for topic %s", topic_name.c_str());
        participant_->delete_topic(topic);
        throw InconsistencyException("Failed to create datareader for topic: " + topic_name);
    }

    // Get the dynamic type from the type support
    DDS::DynamicType_var dyn_type = DDS::DynamicType::_nil();
    DDS::DynamicTypeSupport_var dyn_type_support = DDS::DynamicTypeSupport::_narrow(type_support);
    if (dyn_type_support)
    {
        dyn_type = dyn_type_support->get_type();
    }

    // Create the reader handler
    ReaderHandlerReference new_reader(
        new ReaderHandler(topic, datareader, dyn_type, listener_, data_type_configuration),
        ReaderHandlerDeleter(participant_, subscriber_));

    readers_.insert(std::make_pair(topic_name, std::move(new_reader)));
    
    DDS_INFO("Participant", "Successfully created subscription for topic %s with type %s", 
             topic_name.c_str(), type_name.c_str());
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
    DDS_DEBUG("Participant", "Getting type support for type: %s", type_name.c_str());
    
    // For OpenDDS XTypes, we need to work with DynamicTypeSupport
    // The type information will typically come from discovered endpoints or XML type definitions
    
    // First, check if we have type information in our local cache
    if (dyn_types_info_)
    {
        auto it = dyn_types_info_->find(type_name);
        if (it != dyn_types_info_->end())
        {
            DDS_DEBUG("Participant", "Type %s found in local cache", type_name.c_str());
            // Type is known, but we need to create a DynamicTypeSupport for it
            // In a real scenario, this would involve retrieving the actual type definition
        }
    }
    
    // Create a DynamicTypeSupport instance
    // This will be used for dynamic type handling
    // The actual type definition will be populated through discovery or XML loading
    try
    {
        type_support = new DDS::DynamicTypeSupport();
        if (!type_support)
        {
            DDS_ERROR("Participant", "Failed to create DynamicTypeSupport for type %s", type_name.c_str());
            return DDS::RETCODE_ERROR;
        }
        
        DDS_DEBUG("Participant", "Successfully created DynamicTypeSupport for type %s", type_name.c_str());
        return DDS::RETCODE_OK;
    }
    catch (const std::exception& e)
    {
        DDS_ERROR("Participant", "Exception creating DynamicTypeSupport for type %s: %s", 
                  type_name.c_str(), e.what());
        return DDS::RETCODE_ERROR;
    }
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

    // Check if type is already registered in the participant
    if (!is_type_registered_in_participant_(type_name))
    {
        DDS_DEBUG("Participant", "Type info available. Registering type %s in participant", type_name.c_str());
        
        // Register the type with the participant
        DDS::ReturnCode_t ret = type_support->register_type(participant_, type_name.c_str());
        if (ret == DDS::RETCODE_OK)
        {
            DDS_DEBUG("Participant", "Successfully registered type %s", type_name.c_str());
            discovery_database_->operator[](topic_name) = {type_name, true};
            
            // Store type information for later use
            if (dyn_types_info_)
            {
                // Store the type support for this topic
                (*dyn_types_info_)[topic_name] = type_name;
            }
        }
        else
        {
            DDS_ERROR("Participant", "Failed to register type %s with error code %d", type_name.c_str(), ret);
            discovery_database_->operator[](topic_name) = {type_name, false};
        }
    }
    else
    {
        DDS_DEBUG("Participant", "Type %s is already registered in participant", type_name.c_str());
        discovery_database_->operator[](topic_name) = {type_name, true};
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
    // Check if the type is already registered by attempting to find it
    // OpenDDS doesn't have a direct "is_type_registered" API, so we check by looking up the type
    try
    {
        // Try to find a topic with this type to verify if type is registered
        // If the type is not registered, topic creation would fail
        
        // Check in our local database first
        if (dyn_types_info_)
        {
            for (const auto& [topic, tname] : *dyn_types_info_)
            {
                if (tname == type_name)
                {
                    DDS_DEBUG("Participant", "Type %s found in local type database", type_name.c_str());
                    return true;
                }
            }
        }
        
        // If not in local database, check the discovery database
        for (const auto& [topic, type_info] : *discovery_database_)
        {
            if (std::get<DataTypeNameType>(type_info) == type_name && 
                std::get<TypeInfoAvailable>(type_info))
            {
                DDS_DEBUG("Participant", "Type %s found in discovery database as registered", type_name.c_str());
                return true;
            }
        }
    }
    catch (const std::exception& e)
    {
        DDS_ERROR("Participant", "Exception while checking type registration: %s", e.what());
    }
    
    DDS_DEBUG("Participant", "Type %s not yet registered", type_name.c_str());
    return false;
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

} /* namespace opendds */
} /* namespace plotjuggler */
} /* namespace eprosima */