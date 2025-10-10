#include <dds/DCPS/BuiltInTopicUtils.h>
#include <dds/DCPS/XTypes/DynamicTypeSupport.h>
#include <iostream>

#include "dds_data.hpp"
#include "dds_manager.h"
#include "publication_monitor.hpp"

namespace airbotix {
namespace plotjuggler {
namespace opendds {

//------------------------------------------------------------------------------
PublicationMonitor::PublicationMonitor(DDS::DomainParticipant_ptr participant)
    : m_dataReader(nullptr)
    , participant_(participant)
{
    DDS::Subscriber_var subscriber = participant_->get_builtin_subscriber();
    if (!subscriber) {
        throw std::runtime_error("PublicationMonitor: get_builtin_subscriber failed!");
    }

    m_dataReader = subscriber->lookup_datareader(OpenDDS::DCPS::BUILT_IN_PUBLICATION_TOPIC);
    if (!m_dataReader) {
        throw std::runtime_error("PublicationMonitor: Unable to find built-in publication topic reader");
    }

    m_dataReader->set_listener(this, DDS::DATA_AVAILABLE_STATUS);
}

PublicationMonitor::~PublicationMonitor()
{
    // No explicit deletion needed for m_dataReader
    m_dataReader = nullptr;
}

void PublicationMonitor::set_new_topic_callback(std::function<void(const std::string&, const std::string&)> callback)
{
    m_newTopicCallback = std::move(callback);
}

void PublicationMonitor::on_data_available(DDS::DataReader_ptr reader)
{
    DDS::SampleInfoSeq infoSeq;
    DDS::PublicationBuiltinTopicDataSeq msgList;

    DDS::PublicationBuiltinTopicDataDataReader_var dataReader =
        DDS::PublicationBuiltinTopicDataDataReader::_narrow(reader);

    if (!dataReader) {
        std::cerr << "PublicationMonitor::on_data_available: Error calling _narrow" << std::endl;
        return;
    }

    dataReader->take(
        msgList,
        infoSeq,
        DDS::LENGTH_UNLIMITED,
        DDS::ANY_SAMPLE_STATE,
        DDS::ANY_VIEW_STATE,
        DDS::ALIVE_INSTANCE_STATE);

    for (CORBA::ULong i = 0; i < msgList.length(); ++i) {
        const DDS::PublicationBuiltinTopicData& sampleData = msgList[i];
        const DDS::SampleInfo& sampleInfo = infoSeq[i];

        if (!sampleInfo.valid_data) continue;

        const char* topicNameC = sampleData.topic_name;
        const char* typeNameC = sampleData.type_name;
        if (!topicNameC) continue;
        if (!typeNameC) continue;

        std::string topicName(topicNameC);
        std::string typeName(typeNameC);
        size_t userDataSize = sampleData.topic_data.value.length();

        std::shared_ptr<TopicInfo> topicInfo = CommonData::getTopicInfo(topicName);

        if (topicInfo) {
            if (topicInfo->typeCode() == nullptr && userDataSize > 0) {
                topicInfo->storeUserData(sampleData.topic_data.value);
            }
            if (!topicInfo->dynamicType()) {
                DDS::DynamicType_var dt;
                if (get_dynamic_type(dt, sampleData.key, sampleData.topic_name, sampleData.type_name)) {
                    topicInfo->dynamicType() = dt;
                }
            }

            topicInfo->addPartitions(sampleData.partition);
            if (m_newTopicCallback) {
                m_newTopicCallback(topicName, typeName);
            }
            continue;
        }

        // New topic
        topicInfo = std::make_shared<TopicInfo>();
        topicInfo->topicName() = topicName;
        topicInfo->typeName() = typeName;

        topicInfo->setDurabilityPolicy(sampleData.durability);
        topicInfo->setDeadlinePolicy(sampleData.deadline);
        topicInfo->setLatencyBudgePolicy(sampleData.latency_budget);
        topicInfo->setLivelinessPolicy(sampleData.liveliness);
        topicInfo->setReliabilityPolicy(sampleData.reliability);
        topicInfo->setOwnershipPolicy(sampleData.ownership);
        topicInfo->setDestinationOrderPolicy(sampleData.destination_order);
        topicInfo->setPresentationPolicy(sampleData.presentation);
        topicInfo->addPartitions(sampleData.partition);
        topicInfo->fixHistory();

        DDS::DynamicType_var dt;
        if (get_dynamic_type(dt, sampleData.key, sampleData.topic_name, sampleData.type_name)) {
            topicInfo->dynamicType() = dt;
        }

        if (userDataSize > 0) {
            topicInfo->storeUserData(sampleData.topic_data.value);
        }

        CommonData::storeTopicInfo(topicName, topicInfo);

        if (m_newTopicCallback) {
            m_newTopicCallback(topicName, typeName);
        }
    }

    dataReader->return_loan(msgList, infoSeq);
}

bool PublicationMonitor::get_dynamic_type(DDS::DynamicType_var& type,
                                          const DDS::BuiltinTopicKey_t& key,
                                          const char* topic_name,
                                          const char* type_name)
{
    auto participant = dynamic_cast<OpenDDS::DCPS::DomainParticipantImpl*>(participant_);

    if (!participant) {
        std::cerr << "PublicationMonitor::get_dynamic_type: invalid DomainParticipantImpl" << std::endl;
        return false;
    }

    DDS::ReturnCode_t ret = participant->get_dynamic_type(type, key);
    if (ret != DDS::RETCODE_OK) {
        std::cerr << "Warning: get_dynamic_type for topic " << topic_name
                  << " returned " << OpenDDS::DCPS::retcode_to_string(ret) << std::endl;
        return false;
    }

    DDS::TypeSupport_var dts = new DDS::DynamicTypeSupport(type);
    dts->register_type(participant, type_name);
    return true;
}

} // namespace opendds
} // namespace plotjuggler
} // namespace airbotix