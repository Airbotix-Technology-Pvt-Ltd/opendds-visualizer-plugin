#ifndef _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_OPENDDS_DDS_DATA_HPP_
#define _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_OPENDDS_DDS_DATA_HPP_

#ifdef WIN32
#pragma warning(push, 0)  // No DDS warnings
#endif

#include <dds/DCPS/Serializer.h>
#include <dds/DdsDcpsCoreC.h>
#include <dds/DdsDynamicDataC.h>
#include <tao/AnyTypeCode/TypeCode.h>
#include "dds_manager.h"

#ifdef WIN32
#pragma warning(pop)
#endif

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>
#include <cstdint>
#include <variant>

namespace airbotix {
namespace plotjuggler {
namespace opendds {

class OpenDynamicData;

const std::string DATA_READER_NAME = "DDSMon";

const std::string DURABILITY_QOS_POLICY_STRINGS[] = {
    "VOLATILE",
    "TRANSIENT_LOCAL",
    "TRANSIENT",
    "PERSISTENT"
};

const std::string RELIABILITY_QOS_POLICY_STRINGS[] = {
    "BEST_EFFORT",
    "RELIABLE"
};

const std::string OWNERSHIP_QOS_POLICY_STRINGS[] = {
    "SHARED",
    "EXCLUSIVE"
};

const std::string DESTINATION_ORDER_QOS_POLICY_STRING[] = {
    "BY_RECEPTION",
    "BY_SOURCE"
};

const std::string DESTINATION_SCOPE_QOS_POLICY_STRING[] = {
    "BY_INSTANCE",
    "BY_TOPIC"
};

enum class TypeDiscoveryMode {
    TypeCode,
    DynamicType
};

class TopicInfo {
public:
    TopicInfo();
    ~TopicInfo();

    void addPartitions(const DDS::PartitionQosPolicy& partitionQos);
    void setDurabilityPolicy(const DDS::DurabilityQosPolicy& policy);
    void setDurabilityServicePolicy(const DDS::DurabilityServiceQosPolicy& policy);
    void setDeadlinePolicy(const DDS::DeadlineQosPolicy& policy);
    void setLatencyBudgePolicy(const DDS::LatencyBudgetQosPolicy& policy);
    void setLifespanPolicy(const DDS::LifespanQosPolicy& policy);
    void setLivelinessPolicy(const DDS::LivelinessQosPolicy& policy);
    void setReliabilityPolicy(const DDS::ReliabilityQosPolicy& policy);
    void setOwnershipPolicy(const DDS::OwnershipQosPolicy& policy);
    void setDestinationOrderPolicy(const DDS::DestinationOrderQosPolicy& policy);
    void setPresentationPolicy(const DDS::PresentationQosPolicy& policy);
    void fixHistory();
    void storeUserData(const DDS::OctetSeq& userData);

    // Accessors
    const std::string& topicName() const { return m_topicName; }
    std::string& topicName() { return m_topicName; }
    const std::string& typeName() const { return m_typeName; }
    std::string& typeName() { return m_typeName; }
    const DDS::TopicQos& topicQos() const { return m_topicQos; }
    DDS::PublisherQos& pubQos() { return m_pubQos; }
    const DDS::DataWriterQos& writerQos() const { return m_writerQos; }
    DDS::SubscriberQos& subQos() { return m_subQos; }
    const DDS::DataReaderQos& readerQos() const { return m_readerQos; }
    OpenDDS::DCPS::Extensibility extensibility() const { return m_extensibility; }
    void setExtensibility(OpenDDS::DCPS::Extensibility ext) { m_extensibility = ext; }
    TypeDiscoveryMode typeMode() const { return m_typeMode; }
    void typeMode(TypeDiscoveryMode mode) { m_typeMode = mode; }
    const std::vector<std::string>& partitions() const { return m_partitions; }
    bool hasKey() const { return m_hasKey; }
    CORBA::TypeCode_var typeCode() const { return m_typeCode; }
    void setTypeCode(CORBA::TypeCode_var tc) { m_typeCode = tc; }
    DDS::DynamicType_var dynamicType() const { return m_dynamicType; }
    DDS::DynamicType_var& dynamicType() { return m_dynamicType; }

    // Added for compatibility
    DDS::Topic_var topic;
    DDS::DataReader_var reader;
    OpenDDS::DCPS::Recorder* recorder = nullptr;

private:
    void dumpTypeCode(const char* cdrBuffer, size_t typeCodeSize) const;

    std::string m_topicName;
    std::string m_typeName;
    DDS::TopicQos m_topicQos;
    DDS::PublisherQos m_pubQos;
    DDS::DataWriterQos m_writerQos;
    DDS::SubscriberQos m_subQos;
    DDS::DataReaderQos m_readerQos;
    std::vector<std::string> m_partitions;
    OpenDDS::DCPS::Extensibility m_extensibility;
    bool m_hasKey;
    TypeDiscoveryMode m_typeMode;
    size_t m_typeCodeLength;
    CORBA::TypeCode_var m_typeCode;
    std::unique_ptr<CORBA::Any> m_typeCodeObj;
    DDS::DynamicType_var m_dynamicType;
};

using DDSSampleVariant = std::variant<
    bool,        // TK_BOOLEAN
    uint8_t,     // TK_BYTE, TK_CHAR8
    int16_t,     // TK_INT16
    uint16_t,    // TK_UINT16
    int32_t,     // TK_INT32
    uint32_t,    // TK_UINT32
    int64_t,     // TK_INT64
    uint64_t,    // TK_UINT64
    float,       // TK_FLOAT32
    double,      // TK_FLOAT64
    char,        // TK_CHAR8
    wchar_t,     // TK_CHAR16
    std::string  // TK_STRING8, TK_ENUM
>;

class CommonData {
public:
    static std::unique_ptr<DDSManager> m_ddsManager;
    
    static const int MAX_SAMPLES = 500;

    static void cleanup();
    static void storeTopicInfo(const std::string& topicName, std::shared_ptr<TopicInfo> info);
    static std::shared_ptr<TopicInfo> getTopicInfo(const std::string& topicName);
    static DDSSampleVariant readValue(const std::string& topicName,
                                     const std::string& memberName,
                                     unsigned int index = 0);
    static void flushSamples(const std::string& topicName);
    static void storeSample(const std::string& topicName,
                            const std::string& sampleName,
                            const std::shared_ptr<OpenDynamicData> sample);
    static void storeDynamicSample(const std::string& topicName,
                                  const std::string& sampleName,
                                  DDS::DynamicData_var sample);
    static std::shared_ptr<OpenDynamicData> copySample(const std::string& topicName, const std::string& timestamp);
    static DDS::DynamicData_var copyDynamicSample(const std::string& topicName, const std::string& timestamp);
    static std::vector<std::string> getSampleList(const std::string& topicName);

private:
    static DDSSampleVariant readMember(const std::string& topicName,
                                      const std::string& memberName,
                                      unsigned int index = 0);
    static DDSSampleVariant readDynamicMember(const std::string& topicName,
                                             const std::string& memberName,
                                             unsigned int index = 0);
    static void flushStaticSamples(const std::string& topicName);
    static void flushDynamicSamples(const std::string& topicName);

    
    using SampleMap = std::map<std::string, std::map<std::string, std::shared_ptr<OpenDynamicData>>>;
    static SampleMap m_samples;
    using DynamicSampleMap = std::map<std::string, std::map<std::string, DDS::DynamicData_var>>;
    static DynamicSampleMap m_dynamicSamples;
    using SampleTimeMap = std::map<std::string, std::vector<std::string>>;
    static SampleTimeMap m_sampleTimes;
    using TopicInfoMap = std::map<std::string, std::shared_ptr<TopicInfo>>;
    static TopicInfoMap m_topicInfo;

    static std::mutex m_sampleMutex;
    static std::mutex m_topicMutex;
};

} // namespace opendds
} // namespace plotjuggler
} // namespace airbotix

#endif // _AIRBOTIX_PLOTJUGGLEROPENDDSPLUGIN_PLUGINS_OPENDDS_DDS_DATA_HPP_