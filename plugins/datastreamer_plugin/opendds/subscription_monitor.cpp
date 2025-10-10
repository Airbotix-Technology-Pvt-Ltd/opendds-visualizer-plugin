#include "lib/subscription_monitor.h"
#include "lib/dds_data.h"
#include "dds_manager.h"

#include <dds/DCPS/BuiltInTopicUtils.h>

#include <iostream>
#include <stdexcept>

SubscriptionMonitor::SubscriptionMonitor(DDS::DomainParticipant_ptr participant)
    : m_dataReader(nullptr)
    , participant_(participant)
{
    DDS::Subscriber_var subscriber = participant_->get_builtin_subscriber();
    if (!subscriber) {
        throw std::runtime_error("SubscriptionMonitor: get_builtin_subscriber failed!");
    }

    m_dataReader = subscriber->lookup_datareader(OpenDDS::DCPS::BUILT_IN_SUBSCRIPTION_TOPIC);

    if (!m_dataReader) {
        throw std::runtime_error("SubscriptionMonitor: Unable to find built-in subscription topic reader");
    }

    // Attach a listener which reports subscription info
    m_dataReader->set_listener(this, DDS::DATA_AVAILABLE_STATUS);
}

SubscriptionMonitor::~SubscriptionMonitor() {
    // No explicit delete on m_dataReader.
    m_dataReader = nullptr;
}

void SubscriptionMonitor::set_new_topic_callback(std::function<void(const std::string&)> cb) {
    m_newTopicCallback = std::move(cb);
}

void SubscriptionMonitor::on_data_available(DDS::DataReader_ptr reader) {
    DDS::SampleInfoSeq infoSeq;
    DDS::SubscriptionBuiltinTopicDataSeq msgList;

    DDS::SubscriptionBuiltinTopicDataDataReader_var dataReader =
        DDS::SubscriptionBuiltinTopicDataDataReader::_narrow(reader);

    if (!dataReader) {
        std::cerr << "SubscriptionMonitor::on_data_available: Error calling _narrow" << std::endl;
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
        const DDS::SubscriptionBuiltinTopicData& sampleData = msgList[i];
        const DDS::SampleInfo& sampleInfo = infoSeq[i];

        if (!sampleInfo.valid_data) continue;

        const char* topicNameC = sampleData.topic_name;
        if (!topicNameC) continue;

        std::string topicName(topicNameC);
        size_t userDataSize = sampleData.topic_data.value.length();

        std::shared_ptr<TopicInfo> topicInfo = CommonData::getTopicInfo(topicName);

        if (topicInfo != nullptr && topicInfo->typeCode() == nullptr && userDataSize > 0) {
            topicInfo->storeUserData(sampleData.topic_data.value);
        }

        if (topicInfo != nullptr) {
            topicInfo->addPartitions(sampleData.partition);
            continue;
        }

        topicInfo = std::make_shared<TopicInfo>();
        topicInfo->topicName() = topicName;
        topicInfo->typeName() = sampleData.type_name;

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

        if (userDataSize > 0) {
            topicInfo->storeUserData(sampleData.topic_data.value);
        }

        CommonData::storeTopicInfo(topicName, topicInfo);

        // Notify listener about new topic if callback set
        if (m_newTopicCallback) {
            m_newTopicCallback(topicName);
        }
    }

    dataReader->return_loan(msgList, infoSeq);
}
