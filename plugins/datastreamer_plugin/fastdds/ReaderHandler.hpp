#ifndef _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_READERHANDLER_HPP_
#define _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_READERHANDLER_HPP_

#include <atomic>
#include <nlohmann/json.hpp>
#include <dds/DCPS/DataReaderImpl.h>
#include <dds/DCPS/TopicImpl.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DCPS/XTypes/DynamicDataImpl.h>
#include <dds/DCPS/XTypes/DynamicTypeImpl.h>
#include "FastDdsListener.hpp"
#include "utils/DataTypeConfiguration.hpp"
#include "utils/dynamic_types_utils.hpp"
#include "utils/types.hpp"
#include "utils/Logger.cpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

struct ReaderHandler : public OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
{
public:
    ReaderHandler(
        DDS::Topic_var topic,
        DDS::DataReader_var datareader,
        DDS::DynamicType_ptr type,
        FastDdsListener* listener,
        const DataTypeConfiguration& data_type_configuration);

    virtual ~ReaderHandler();

    ReaderHandler& operator=(ReaderHandler&& other);
    ReaderHandler& operator=(const ReaderHandler& other);

    void stop();

    void on_data_available(DDS::DataReader* reader) override;

    std::string topic_name() const;  // Changed to std::string
    std::string type_name() const;   // Changed to std::string

    std::vector<types::DatumLabel> numeric_data_series_names() const;
    std::vector<types::DatumLabel> string_data_series_names() const;

protected:
    void create_data_structures_(DDS::DynamicData_ptr data = nullptr);

    static DDS::StatusMask default_listener_mask_();

    FastDdsListener* listener_;
    DDS::Topic_var topic_;
    DDS::DataReader_var reader_;
    DDS::DynamicType_ptr type_;
    DDS::DynamicData_ptr data_;
    std::atomic<bool> stop_;
    utils::TypeIntrospectionNumericStruct numeric_data_info_;
    utils::TypeIntrospectionStringStruct string_data_info_;
    DataTypeConfiguration data_type_configuration_;
};

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLERFASTDDSPLUGIN_PLUGINS_DATASTREAMERPLUGIN_FASTDDS_READERHANDLER_HPP_