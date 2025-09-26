// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Visualizer Plugin.
//
// eProsima Fast DDS Visualizer Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Visualizer Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Visualizer Plugin. If not, see <https://www.gnu.org/licenses/>.

/**
 * @file ReaderHandler.hpp
 */

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

/**
 * @brief TODO
 *
 */
struct ReaderHandler : public OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
{
public:

    ////////////////////////////////////////////////////
    // CREATION & DESTRUCTION
    ////////////////////////////////////////////////////

ReaderHandler(
        DDS::Topic_var topic,
        DDS::DataReader_var datareader,
        DDS::DynamicType_ptr type,
        FastDdsListener* listener,
        const DataTypeConfiguration& data_type_configuration);


    virtual ~ReaderHandler();

    ReaderHandler& operator =(
            ReaderHandler&& other);

    ReaderHandler& operator =(
            const ReaderHandler& other);

    ////////////////////////////////////////////////////
    // INTERACTION METHODS
    ////////////////////////////////////////////////////

    void stop();


    ////////////////////////////////////////////////////
    // LISTENER [ DATAREADER ] METHODS
    ////////////////////////////////////////////////////

    void on_data_available(
            DDS::DataReader* reader) override;


    ////////////////////////////////////////////////////
    // VALUES METHODS
    ////////////////////////////////////////////////////

    const std::string& topic_name() const;

    const std::string& type_name() const;

    std::vector<types::DatumLabel> numeric_data_series_names() const;

    std::vector<types::DatumLabel> string_data_series_names() const;

    ////////////////////////////////////////////////////
    // AUXILIAR METHODS
    ////////////////////////////////////////////////////

    void create_data_structures_(
            DDS::DynamicData_ptr data = nullptr);

    ////////////////////////////////////////////////////
    // AUXILIAR STATIC METHODS
    ////////////////////////////////////////////////////

    /**
     * @brief Get default mask
     *
     * Callbacks accepted by this mask:
     * - on_data_available
     *
     * @return DDS::StatusMask with callbacks needed
     */
    static DDS::StatusMask default_listener_mask_();


    ////////////////////////////////////////////////////
    // INTERNAL VARIABLES
    ////////////////////////////////////////////////////

    FastDdsListener* listener_;


    ////////////////////////////////////////////////////
    // FAST DDS POINTERS
    ////////////////////////////////////////////////////

    //! Topic related with this DataReader
    DDS::Topic_var topic_;

    //! DataReader
    DDS::DataReader_var reader_;

    //! Type Informantion
    DDS::DynamicType_ptr type_;

    //! Data Type element
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
