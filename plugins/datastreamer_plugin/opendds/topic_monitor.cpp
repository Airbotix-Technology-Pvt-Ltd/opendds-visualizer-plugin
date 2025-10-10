#include "topic_monitor.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// ----- TopicMonitor implementation ----- 

namespace airbotix {
namespace plotjuggler {
namespace opendds {

TopicMonitor::TopicMonitor(const std::string& topicName, DDS::DomainParticipant_ptr participant, OpenDdsListener* listener, const DataTypeConfiguration& data_type_configuration)
    : participant_(participant)
    , m_topicName(topicName)
    , m_recorder_listener(OpenDDS::DCPS::make_rch<RecorderListener>(OpenDDS::DCPS::ref(*this)))
    , m_recorder(nullptr)
    , m_dr_listener(new DataReaderListenerImpl(*this))
    , m_topic(nullptr)
    , m_paused(false)
    , listener_(listener)
    , data_type_configuration_(data_type_configuration)
{
    // Make sure we have an information object for this topic
    std::shared_ptr<TopicInfo> topicInfo = CommonData::getTopicInfo(topicName);
    if (topicInfo == nullptr)
    {
        throw std::runtime_error(std::string("Unable to find topic information for topic \"") + topicName + "\"");
    }

    // Store extensibility
    m_extensibility = topicInfo->extensibility();
    OpenDDS::DCPS::Service_Participant* service = TheServiceParticipant;

    if (!participant_)
    {
        throw std::runtime_error("No domain participant_");
    }

    if (topicInfo->typeCode())
    {
        // Use the existing mechanism based on TypeCode.
        m_typeCode = topicInfo->typeCode();
        m_topic = service->create_typeless_topic(participant_,
                                                 topicInfo->topicName().c_str(),
                                                 topicInfo->typeName().c_str(),
                                                 topicInfo->hasKey(),
                                                 topicInfo->topicQos(),
                                                 new GenericTopicListener,
                                                 DDS::INCONSISTENT_TOPIC_STATUS);
        if (!m_topic)
        {
            throw std::runtime_error(std::string("Failed to create typeless topic \"") + topicInfo->topicName() + "\"");
        }

        m_recorder = service->create_recorder(participant_,
                                              m_topic,
                                              topicInfo->subQos(),
                                              topicInfo->readerQos(),
                                              m_recorder_listener);
        if (!m_recorder)
        {
            throw std::runtime_error(std::string("Failed to create recorder for topic \"") + topicInfo->topicName() + "\"");
        }

        DDS_DEBUG("TopicMonitor", "Created recorder for topic \"%s\"", topicInfo->topicName().c_str());
    }
    else
    {
        // Use DynamicDataReader instead.
        // When this is called, the information about this topic including its
        // DynamicType should be already obtained. The topic's type should also be
        // registered with the local domain participant_->
        m_topic = participant_->create_topic(topicInfo->topicName().c_str(),
                                            topicInfo->typeName().c_str(),
                                            topicInfo->topicQos(),
                                            0,
                                            0);
        if (!m_topic)
        {
            throw std::runtime_error(std::string("Failed to create topic \"") + topicInfo->topicName() + "\"");
        }

        DDS::Subscriber_var subscriber = participant_->create_subscriber(topicInfo->subQos(),
                                                                        0,
                                                                        0);
        if (!subscriber)
        {
            throw std::runtime_error(std::string("Failed to create subscriber for topic \"") + topicInfo->topicName() + "\"");
        }

        m_dr = subscriber->create_datareader(m_topic,
                                             topicInfo->readerQos(),
                                             m_dr_listener,
                                             OpenDDS::DCPS::DEFAULT_STATUS_MASK);
        if (!m_dr)
        {
            throw std::runtime_error(std::string("Failed to create data reader for topic \"") + topicInfo->topicName() + "\"");
        }
        topicInfo->typeMode(TypeDiscoveryMode::DynamicType);

        DDS_DEBUG("TopicMonitor", "Created data reader for topic \"%s\"", topicInfo->topicName().c_str());
    }
}

TopicMonitor::~TopicMonitor()
{
    close();
}

void TopicMonitor::close()
{
    m_paused = true;

    if (m_recorder) {
        OpenDDS::DCPS::Service_Participant* service = TheServiceParticipant;
        service->delete_recorder(m_recorder);
        m_recorder = nullptr;
    }

    if (participant_ && m_topic) {
        participant_->delete_topic(m_topic);
    }
    m_topic = nullptr;
}

void TopicMonitor::on_sample_data_received(OpenDDS::DCPS::Recorder*,
                                           const OpenDDS::DCPS::RawDataSample& rawSample)
{
    if (m_paused)
    {
        return;
    }

    DDS_DEBUG_ONCE("TopicMonitor", "on_sample_data_received");

    OpenDDS::DCPS::Encoding::Kind globalEncoding = QosDictionary::getEncodingKind();

    if (rawSample.header_.message_id_ != OpenDDS::DCPS::SAMPLE_DATA)
    {
        std::cerr << "\nSkipping message that is not SAMPLE_DATA. This should not be possible! "
                  << "Check for compatibility with RecorderImpl::data_received()."
                  << std::endl;
        return;
    }

    if (globalEncoding != rawSample.encoding_kind_)
    {
        std::cerr << "Skipping message with encoding kind that does not match our encoding kind.\n"
                  << "Global Encoding Kind: " << OpenDDS::DCPS::Encoding::kind_to_string(globalEncoding) << "\n"
                  << "Raw Sample Encoding Kind: " << OpenDDS::DCPS::Encoding::kind_to_string(rawSample.encoding_kind_) << "\n"
                  << "There probably is a mismatch between the configuration of ddsmon and one or more publishers. "
                  << "Either the UseXTypes flag in opendds.ini or DDS_USE_OLD_CDR environment variable."
                  << std::endl;
        return;
    }

    OpenDDS::DCPS::Message_Block_Ptr mbCopy(rawSample.sample_->duplicate());
    OpenDDS::DCPS::Serializer serial(
        rawSample.sample_.get(), rawSample.encoding_kind_, static_cast<OpenDDS::DCPS::Endianness>(rawSample.header_.byte_order_));

    //RJ 2022-01-20 With OpenDDS 3.19.0, the entire message header is read before the sample gets passed to this function.
    //Code that strips off the RTPS header has been removed.
    //Same with the reset_alignment call in the serializer. That has already happened before the sample is passed to this function.

    std::shared_ptr<OpenDynamicData> sample = CreateOpenDynamicData(m_typeCode, globalEncoding, m_extensibility);
    if (globalEncoding != OpenDDS::DCPS::Encoding::KIND_XCDR1)
    {
        std::cout << "Removing delimiter header" << std::endl;
        uint32_t delim_header = 0;
        if (!(serial >> delim_header))
        {
            std::cerr << "TopicMonitor::on_sample_data_received: Could not read stream delimiter" << std::endl;
            return;
        }
    }

    (*(sample.get())) << serial;
    //sample->dump();

    // Get timestamp
    double timestamp = utils::get_timestamp_seconds_numeric_value(rawSample.source_timestamp_);

    // serialize data
    nlohmann::json serialized_data = parse_dynamic_data(sample);

    // Update listener
    update_listener(timestamp, serialized_data);
}

void TopicMonitor::on_data_available(DDS::DataReader_ptr dr)
{
    if (m_paused) return;

    DDS_DEBUG_ONCE("TopicMonitor", "on_data_available for topic \"%s\"", m_topicName.c_str());

    DDS::DynamicDataReader_var ddr = DDS::DynamicDataReader::_narrow(dr);
    DDS::DynamicDataSeq messages;
    DDS::SampleInfoSeq infos;

    DDS::ReturnCode_t ret = ddr->take(messages, infos, DDS::LENGTH_UNLIMITED,
                                     DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
    if (ret != DDS::RETCODE_OK && ret != DDS::RETCODE_NO_DATA) {
        std::cerr << "Failed to take samples for topic " << m_topicName << std::endl;
        return;
    }

    for (unsigned int i = 0; i < messages.length(); ++i) {
        if (infos[i].valid_data) {

            // Get timestamp
            double timestamp = utils::get_timestamp_seconds_numeric_value(infos[i].source_timestamp);

            // serialize data
            nlohmann::json serialized_data = parse_dynamic_data(DDS::DynamicData::_duplicate(messages[i].in()));

            // Update listener
            update_listener(timestamp, serialized_data);
        }
    }
}

void TopicMonitor::pause()
{
    m_paused = true;
}

void TopicMonitor::unpause()
{
    m_paused = false;
}
////////////////////////////////////////////////////
// AUXILIAR METHODS
////////////////////////////////////////////////////
void TopicMonitor::update_listener(double timestamp, const nlohmann::json& data)
{
    // Reset stored data info
    numeric_data_info_.clear();
    string_data_info_.clear();

    // Format the data received to show it in the GUI
    create_data_structures_(data);

    // Update previous data view according to new received data structure
    listener_->on_data_available();

    // Get value maps from data and send callback if there are data
    if (!numeric_data_info_.empty())
    {
        listener_->on_double_data_read(
            numeric_data_info_,
            timestamp);
    }

    // Same for strings
    if (!string_data_info_.empty())
    {
        listener_->on_string_data_read(
            string_data_info_,
            timestamp);
    }
}

void TopicMonitor::create_data_structures_(
        const nlohmann::json& data)
{
    // Create the structures to store the data introspection information AND the data itself
    utils::get_formatted_data(
        m_topicName,
        data_type_configuration_,
        numeric_data_info_,
        string_data_info_,
        data);

    DEBUG("Completed type introspection created in topic: " << m_topicName << " with types: ");
    for (const auto& info : numeric_data_info_)
    {
        DEBUG("\tNumeric: " << std::get<0>(info));
    }
    for (const auto& info : string_data_info_)
    {
        DEBUG("\tString: " << std::get<0>(info));
    }
}

std::vector<types::DatumLabel> TopicMonitor::numeric_data_series_names() const
{
    return utils::get_introspection_type_names(numeric_data_info_);
}

std::vector<types::DatumLabel> TopicMonitor::string_data_series_names() const
{
    return utils::get_introspection_type_names(string_data_info_);
}

} // namespace opendds
} // namespace plotjuggler
} // namespace airbotix