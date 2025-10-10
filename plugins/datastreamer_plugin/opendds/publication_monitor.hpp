#ifndef __DDS_PUBLICATION_MONITOR_H__
#define __DDS_PUBLICATION_MONITOR_H__

#include "dds_manager.h"
#include <airbotix_base/Logger.hpp>

#include <string>

/**
 * @brief Listener class which receives information about publishers on the bus.
 */
class PublicationMonitor : public virtual GenericReaderListener
{
public:
    PublicationMonitor(DDS::DomainParticipant_ptr partcipant);
    ~PublicationMonitor();

    void on_data_available(DDS::DataReader_ptr reader) override;

    /**
     * @brief Callback setter for new topic discovered notifications.
     */
    void set_new_topic_callback(std::function<void(const std::string&, const std::string&)> callback);

private:
    DDS::DataReader_ptr m_dataReader;

    DDS::DomainParticipant_ptr participant_;

    bool get_dynamic_type(DDS::DynamicType_var& type, const DDS::BuiltinTopicKey_t& key,
                          const char* topic_name, const char* type_name);

    std::function<void(const std::string&)> m_newTopicCallback;
};

#endif // __DDS_PUBLICATION_MONITOR_H__
