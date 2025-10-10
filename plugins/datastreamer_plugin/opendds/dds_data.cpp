#include "dds_data.hpp"
#include "dds_manager.h"
#include "qos_dictionary.h"
#include "open_dynamic_data.hpp"

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/XTypes/Utils.h>
#include <tao/AnyTypeCode/Any.h>

#include <iostream>

namespace airbotix {
namespace plotjuggler {
namespace opendds {

std::unique_ptr<DDSManager> CommonData::m_ddsManager;
std::map<std::string, std::map<std::string, std::shared_ptr<OpenDynamicData>>> CommonData::m_samples;
std::map<std::string, std::vector<std::string>> CommonData::m_sampleTimes;
std::map<std::string, std::shared_ptr<TopicInfo>> CommonData::m_topicInfo;
std::map<std::string, std::map<std::string, DDS::DynamicData_var>> CommonData::m_dynamicSamples;
std::mutex CommonData::m_sampleMutex;
std::mutex CommonData::m_topicMutex;

//------------------------------------------------------------------------------
void CommonData::cleanup() {
    {
        std::lock_guard<std::mutex> locker(m_sampleMutex);
        m_samples.clear();
        m_sampleTimes.clear();
    }

    {
        std::lock_guard<std::mutex> locker(m_topicMutex);
        m_topicInfo.clear();
    }

    m_ddsManager.reset();
}

//------------------------------------------------------------------------------
void CommonData::storeTopicInfo(const std::string& topicName, std::shared_ptr<TopicInfo> info) {
    std::lock_guard<std::mutex> locker(m_topicMutex);
    m_topicInfo[topicName] = info;
}

//------------------------------------------------------------------------------
std::shared_ptr<TopicInfo> CommonData::getTopicInfo(const std::string& topicName) {
    std::lock_guard<std::mutex> locker(m_topicMutex);
    auto it = m_topicInfo.find(topicName);
    return it != m_topicInfo.end() ? it->second : std::shared_ptr<TopicInfo>();
}

//------------------------------------------------------------------------------
void CommonData::flushSamples(const std::string& topicName) {
    std::shared_ptr<TopicInfo> topicInfo = getTopicInfo(topicName);
    if (!topicInfo) {
        return;
    }
    if (topicInfo->typeMode() == TypeDiscoveryMode::TypeCode) {
        flushStaticSamples(topicName);
    } else {
        flushDynamicSamples(topicName);
    }
}

//------------------------------------------------------------------------------
void CommonData::flushStaticSamples(const std::string& topicName) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    m_samples.erase(topicName);
    m_sampleTimes.erase(topicName);
}

//------------------------------------------------------------------------------
void CommonData::flushDynamicSamples(const std::string& topicName) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    m_dynamicSamples.erase(topicName);
    m_sampleTimes.erase(topicName);
}

//------------------------------------------------------------------------------
void CommonData::storeSample(const std::string& topicName,
                             const std::string& sampleName,
                             const std::shared_ptr<OpenDynamicData> sample) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    auto& sampleMap = m_samples[topicName];
    auto& timesList = m_sampleTimes[topicName];

    // Store by timestamp key
    sampleMap[sampleName] = sample;
    timesList.insert(timesList.begin(), sampleName);

    // Cleanup old samples if exceeded MAX_SAMPLES
    while (timesList.size() > MAX_SAMPLES) {
        std::string oldestTime = timesList.back();
        sampleMap.erase(oldestTime);  // Remove from map by key
        timesList.pop_back();         // Remove from time list
    }
}

//------------------------------------------------------------------------------
void CommonData::storeDynamicSample(const std::string& topicName,
                                   const std::string& sampleName,
                                   DDS::DynamicData_var sample) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    auto& sampleMap = m_dynamicSamples[topicName];  // Added missing semicolon
    auto& timesList = m_sampleTimes[topicName];

    // Store by timestamp key
    sampleMap[sampleName] = sample;
    timesList.insert(timesList.begin(), sampleName);

    // Cleanup old samples if exceeded MAX_SAMPLES
    while (timesList.size() > MAX_SAMPLES) {
        std::string oldestTime = timesList.back();
        sampleMap.erase(oldestTime);  // Remove from map by key
        timesList.pop_back();         // Remove from time list
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<OpenDynamicData> CommonData::copySample(const std::string& topicName, const std::string& timestamp) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    
    // Find the topic
    auto topicIt = m_samples.find(topicName);
    if (topicIt == m_samples.end()) {
        return std::shared_ptr<OpenDynamicData>();
    }
    
    // Find the sample by timestamp
    auto sampleIt = topicIt->second.find(timestamp);
    if (sampleIt != topicIt->second.end()) {
        return sampleIt->second;
    }
    
    return std::shared_ptr<OpenDynamicData>();
}

//------------------------------------------------------------------------------
DDS::DynamicData_var CommonData::copyDynamicSample(const std::string& topicName, const std::string& timestamp) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    
    // Find the topic
    auto topicIt = m_dynamicSamples.find(topicName);
    if (topicIt == m_dynamicSamples.end()) {
        return DDS::DynamicData_var();
    }
    
    // Find the sample by timestamp
    auto sampleIt = topicIt->second.find(timestamp);
    if (sampleIt != topicIt->second.end()) {
        return sampleIt->second;
    }
    
    return DDS::DynamicData_var();
}

//------------------------------------------------------------------------------
std::vector<std::string> CommonData::getSampleList(const std::string& topicName) {
    std::lock_guard<std::mutex> locker(m_sampleMutex);
    auto it = m_sampleTimes.find(topicName);
    return it != m_sampleTimes.end() ? it->second : std::vector<std::string>();
}

//------------------------------------------------------------------------------
TopicInfo::TopicInfo()
  : m_topicQos{QosDictionary::Topic::bestEffort()}
  , m_pubQos{QosDictionary::Publisher::defaultQos()}
  , m_writerQos{QosDictionary::DataWriter::bestEffort()}
  , m_subQos{QosDictionary::Subscriber::defaultQos()}
  , m_readerQos{QosDictionary::DataReader::bestEffort()}
  , m_extensibility{OpenDDS::DCPS::Extensibility::APPENDABLE}
  , m_hasKey{true}
  , m_typeMode{TypeDiscoveryMode::TypeCode}
  , m_typeCodeLength{0}
{}

//------------------------------------------------------------------------------
TopicInfo::~TopicInfo() {}

//------------------------------------------------------------------------------
void TopicInfo::addPartitions(const DDS::PartitionQosPolicy& partitionQos) {
    bool changeFound = false;
    const DDS::StringSeq& partitionNames = partitionQos.name;
    for (CORBA::ULong i = 0; i < partitionNames.length(); i++) {
        std::string partitionString(partitionNames[i].in());
        if (std::find(m_partitions.begin(), m_partitions.end(), partitionString) == m_partitions.end()) {
            m_partitions.push_back(partitionString);
            changeFound = true;
        }
    }
    if (changeFound) {
        std::sort(m_partitions.begin(), m_partitions.end());
    }
}

//------------------------------------------------------------------------------
void TopicInfo::setDurabilityPolicy(const DDS::DurabilityQosPolicy& policy) {
    m_topicQos.durability = policy;
    m_writerQos.durability = policy;
    m_readerQos.durability = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setDurabilityServicePolicy(const DDS::DurabilityServiceQosPolicy& policy) {
    m_topicQos.durability_service = policy;
    m_writerQos.durability_service = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setDeadlinePolicy(const DDS::DeadlineQosPolicy& policy) {
    m_topicQos.deadline = policy;
    m_writerQos.deadline = policy;
    m_readerQos.deadline = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setLatencyBudgePolicy(const DDS::LatencyBudgetQosPolicy& policy) {
    m_topicQos.latency_budget = policy;
    m_writerQos.latency_budget = policy;
    m_readerQos.latency_budget = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setLifespanPolicy(const DDS::LifespanQosPolicy& policy) {
    m_topicQos.lifespan = policy;
    m_writerQos.lifespan = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setLivelinessPolicy(const DDS::LivelinessQosPolicy& policy) {
    m_topicQos.liveliness = policy;
    m_writerQos.liveliness = policy;
    m_readerQos.liveliness = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setReliabilityPolicy(const DDS::ReliabilityQosPolicy& policy) {
    m_topicQos.reliability = policy;
    m_writerQos.reliability = policy;
    m_readerQos.reliability = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setOwnershipPolicy(const DDS::OwnershipQosPolicy& policy) {
    m_topicQos.ownership = policy;
    m_writerQos.ownership = policy;
    m_readerQos.ownership = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setDestinationOrderPolicy(const DDS::DestinationOrderQosPolicy& policy) {
    m_topicQos.destination_order = policy;
    m_writerQos.destination_order = policy;
    m_readerQos.destination_order = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::setPresentationPolicy(const DDS::PresentationQosPolicy& policy) {
    m_pubQos.presentation = policy;
    m_subQos.presentation = policy;
}

//------------------------------------------------------------------------------
void TopicInfo::fixHistory() {
    if (m_topicQos.durability.kind == DDS::VOLATILE_DURABILITY_QOS &&
        m_topicQos.reliability.kind == DDS::RELIABLE_RELIABILITY_QOS) {
        m_topicQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
    if (m_readerQos.durability.kind == DDS::VOLATILE_DURABILITY_QOS &&
        m_readerQos.reliability.kind == DDS::RELIABLE_RELIABILITY_QOS) {
        m_readerQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
    if (m_writerQos.durability.kind == DDS::VOLATILE_DURABILITY_QOS &&
        m_writerQos.reliability.kind == DDS::RELIABLE_RELIABILITY_QOS) {
        m_writerQos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
}

//------------------------------------------------------------------------------
void TopicInfo::storeUserData(const DDS::OctetSeq& userData) {
    if (userData.length() == 0 || m_typeCode) {
        return;
    }

    constexpr size_t headerSize = 8;
    const size_t totalSize = userData.length();
    if (totalSize <= headerSize) {
        return;
    }

    const size_t typeCodeSize = totalSize - headerSize;
    char header[headerSize];
    memcpy(header, &userData[0], headerSize);

    if (strncmp(header, "USR", 3) != 0) {
        return;
    }

    this->m_hasKey = (header[5] == 1);
    switch (header[6]) {
    case 0: this->m_extensibility = OpenDDS::DCPS::Extensibility::APPENDABLE; break;
    case 1: this->m_extensibility = OpenDDS::DCPS::Extensibility::FINAL; break;
    case 2: this->m_extensibility = OpenDDS::DCPS::Extensibility::MUTABLE; break;
    default:
        std::cout << "Unknown extensibility value " << static_cast<int>(header[6])
                  << " from topic type \"" << this->m_typeName << "\"" << std::endl;
        return;
    }

    const char* cdrBuffer = reinterpret_cast<const char*>(&userData[headerSize]);
    TAO_InputCDR topicTypeIn(cdrBuffer, typeCodeSize);
    this->m_typeCodeObj = std::make_unique<CORBA::Any>();
    if (!(topicTypeIn >> *this->m_typeCodeObj)) {
        std::cout << "Failed to demarshal topic type \"" << this->m_typeName << "\" from CDR" << std::endl;
        return;
    }

    this->m_typeCodeLength = typeCodeSize;
    this->m_typeCode = this->m_typeCodeObj->type();
}

//------------------------------------------------------------------------------
void TopicInfo::dumpTypeCode(const char* cdrBuffer, size_t typeCodeSize) const {
    printf("\n=== Begin CDR Dump ===\n");
    for (size_t i = 0; i < typeCodeSize + 16; i += 16) {
        printf("%04zX ", i);
        for (size_t j = 0; j < 16; ++j) {
            if (j == 8) printf(" ");
            if (i + j < typeCodeSize) printf("%02X", (uint8_t)cdrBuffer[i + j]);
            else printf("  ");
        }
        printf("   ");
        for (size_t j = 0; j < 16; ++j) {
            if (j == 8) printf(" ");
            if (i + j < typeCodeSize && isprint(cdrBuffer[i + j])) printf("%c", cdrBuffer[i + j]);
            else printf(" ");
        }
        printf("\n");
    }
    printf("\n=== End CDR Dump ===\n");
}

} // namespace opendds
} // namespace plotjuggler
} // namespace airbotix
