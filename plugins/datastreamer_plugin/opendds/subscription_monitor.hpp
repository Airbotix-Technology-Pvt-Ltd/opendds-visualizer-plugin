#ifndef __DDS_SUBSCRIPTION_MONITOR_H__
#define __DDS_SUBSCRIPTION_MONITOR_H__

#include "dds_manager.h"
#include "utils/Logger.hpp"

#include <string>
#include <functional>

namespace airbotix {
namespace plotjuggler {
namespace opendds {

/**
 * @ Listener class which receives information about subscribers on the bus.
 *
 * Not thread safe. Callbacks must be thread safe if used across threads.
 */
class SubscriptionMonitor : public virtual GenericReaderListener
{
public:
    /**
     * @brief Constructor for the DDS subscription monitor class.
     */
    SubscriptionMonitor(DDS::DomainParticipant_ptr participant);

    /**
     * @brief Destructor for the DDS subscription monitor class.
     */
    ~SubscriptionMonitor();

    /**
     * @brief Callback method to handle the DDS::DATA_AVAILABLE_STATUS message.
     * @param[in] reader The data reader containing the new message.
     */
    void on_data_available(DDS::DataReader_ptr reader) override;

    /**
     * @brief Register a callback to be notified of new topic names discovered.
     * @param cb Callback with signature void(const std::string& topicName)
     */
    void set_new_topic_callback(std::function<void(const std::string&)> cb);

private:
                          
    DDS::DataReader_ptr m_dataReader;

    DDS::DomainParticipant_ptr participant_;
    std::function<void(const std::string&)> m_newTopicCallback;
};

} // namespace opendds
} // namespace plotjuggler
} // namespace airbotix

#endif // __DDS_SUBSCRIPTION_MONITOR_H__