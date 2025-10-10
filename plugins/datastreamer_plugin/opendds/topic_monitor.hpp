#ifndef __DDS_TOPIC_MONITOR_H__
#define __DDS_TOPIC_MONITOR_H__

#include <dds/DCPS/TopicDescriptionImpl.h>
#include <dds/DCPS/OwnershipManager.h>
#include <dds/DCPS/EntityImpl.h>
#include <dds/DCPS/RecorderImpl.h>
#include <dds/DdsDcpsCoreC.h>
#include <dds/DCPS/Serializer.h>
#include <tao/AnyTypeCode/TypeCode.h>

#include <airbotix_base/Logger.hpp>

#include <memory>
#include <string>

/**
 * @brief Topic monitor for receiving raw DDS data samples.
 */
class TopicMonitor
{
public:
    /**
     * @brief Constructor for the DDS topic monitor.
     * @param[in] topicName The name of the topic to monitor.
     */
    TopicMonitor(const std::string& topicName, DDS::DomainParticipant_ptr participant, FastDdsListener* listener);

    /**
     * @brief Close the topic monitor.
     */
    void close();

    /**
     * @brief Read all the samples from the data reader.
     * @param[in] recorder The recorder object with the data.
     * @param[in] rawSample The new data sample for this topic.
    */
    void on_sample_data_received(OpenDDS::DCPS::Recorder* recorder,
                                 const OpenDDS::DCPS::RawDataSample& rawSample);

    class RecorderListener : public OpenDDS::DCPS::RecorderListener
    {
    public:
        RecorderListener(TopicMonitor& monitor) : m_monitor(monitor) {}

        void on_sample_data_received(OpenDDS::DCPS::Recorder* recorder,
                                     const OpenDDS::DCPS::RawDataSample& rawSample) override
        {
            m_monitor.on_sample_data_received(recorder, rawSample);
        }

        void on_recorder_matched(OpenDDS::DCPS::Recorder* /*recorder*/,
                                 const DDS::SubscriptionMatchedStatus& /*status*/) override {}
    private:
        TopicMonitor& m_monitor;
    };

    /**
     * @brief Callback for data available in DataReader.
     */
    void on_data_available(DDS::DataReader_ptr reader);

    class DataReaderListenerImpl : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
    {
    public:
      DataReaderListenerImpl(TopicMonitor& monitor) : m_monitor(monitor) {}
      virtual void on_requested_deadline_missed(DDS::DataReader_ptr,
                                                const DDS::RequestedDeadlineMissedStatus&) {}

      virtual void on_requested_incompatible_qos(DDS::DataReader_ptr,
                                                 const DDS::RequestedIncompatibleQosStatus&) {}

      virtual void on_liveliness_changed(DDS::DataReader_ptr,
                                         const DDS::LivelinessChangedStatus&) {}

      virtual void on_subscription_matched(DDS::DataReader_ptr,
                                           const DDS::SubscriptionMatchedStatus&) {}

      virtual void on_sample_rejected(DDS::DataReader_ptr,
                                      const DDS::SampleRejectedStatus&) {}

      virtual void on_data_available(DDS::DataReader_ptr reader)
      {
        m_monitor.on_data_available(reader);
      }

      virtual void on_sample_lost(DDS::DataReader_ptr,
                                  const DDS::SampleLostStatus&) {}
    private:
      TopicMonitor& m_monitor;
    };

    /**
     * @brief Stop receiving samples.
     */
    void pause();

    /**
     * @brief Resume receiving samples.
     */
    void unpause();

    /**
     * @brief Destructor.
     */
    virtual ~TopicMonitor();

private:

    void create_data_structures_(const nlohmann::json& data);

    void update_listener(double timestamp, const nlohmann::json& data)

    std::vector<types::DatumLabel> numeric_data_series_names() const;

    std::vector<types::DatumLabel> string_data_series_names() const;

    DDS::DomainParticipant_ptr participant_;
    std::string m_topicName;

    /// Stores the typecode for this topic.
    CORBA::TypeCode_var m_typeCode;

    /// Listener for the recorder, calls back into this object
    OpenDDS::DCPS::RcHandle<RecorderListener> m_recorder_listener;

    /// Stores the recorder object for this monitor.
    OpenDDS::DCPS::Recorder* m_recorder;

    /// Listener for a dynamic reader
    DDS::DataReaderListener_var m_dr_listener;

    /// A dynamic data reader for this topic
    DDS::DataReader_var m_dr;

    /// Stores the topic object for this monitor.
    DDS::Topic* m_topic;

    /// The paused status of the data reader.
    bool m_paused;

    /// The topic extensibility
    OpenDDS::DCPS::Extensibility m_extensibility;

    utils::TypeIntrospectionNumericStruct numeric_data_info_;
    utils::TypeIntrospectionStringStruct string_data_info_;

    FastDdsListener* listener_;
};

#endif // __DDS_TOPIC_MONITOR_H__
