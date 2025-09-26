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

// #include <fastdds/dds/domain/DomainParticipantFactory.hpp>
// #include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
// #include <fastdds/dds/xtypes/utils.hpp>

#include "ReaderHandler.hpp"
#include "utils/utils.hpp"
#include "utils/dynamic_types_utils.hpp"

namespace eprosima {
namespace plotjuggler {
namespace fastdds {

using namespace DDS;

////////////////////////////////////////////////////
// CREATION & DESTRUCTION
////////////////////////////////////////////////////

ReaderHandler::ReaderHandler(
        DDS::Topic_var topic,
        DDS::DataReader_var datareader,
        DDS::DynamicType_ptr type,
        FastDdsListener* listener,
        const DataTypeConfiguration& data_type_configuration)
    : topic_(topic)
    , reader_(datareader)
    , type_(type)
    , listener_(listener)
    , stop_(false)
    , data_type_configuration_(data_type_configuration)
{
    // Create data so it is not required to create it each time and avoid reallocation if possible
    data_ = DynamicDataFactory::get_instance()->create_data(type_);

    // Set this object as this reader's listener
    reader_->set_listener(this, OpenDDS::DCPS::DEFAULT_STATUS_MASK);
}

ReaderHandler::~ReaderHandler()
{
    // Stop the reader
    stop();

    // Delete created data
    DynamicDataFactory::get_instance()->delete_data(data_);
}

////////////////////////////////////////////////////
// INTERACTION METHODS
////////////////////////////////////////////////////

void ReaderHandler::stop()
{
    // Stop the reader
    stop_ = true;
    reader_->set_listener(nullptr, OpenDDS::DCPS::DEFAULT_STATUS_MASK);
}

////////////////////////////////////////////////////
// LISTENER METHODS [ DATAREADER ]
////////////////////////////////////////////////////

void ReaderHandler::on_data_available(DDS::DataReader_ptr reader)
{
  DDS::ReturnCode_t rc = DDS::RETCODE_OK;
  DDS::SampleInfo info;

  // Create a dynamic data instance from a DynamicType_var 'type_' you already have.
  // Note: DDS::DynamicData_var is a CORBA _var smart pointer.
  DDS::DynamicData_var data = new DynamicData(type_);

  // Loop until no more data or you decide to stop
  while (!stop_ && rc == DDS::RETCODE_OK) {
    // take_next_sample has an overload that accepts DDS::DynamicData_var / pointer
    rc = reader->take_next_sample(data.inout(), info);

    if (rc == DDS::RETCODE_OK &&
        info.instance_state == DDS::ALIVE_INSTANCE_STATE &&
        info.valid_data) {

      // Use 'data' (DDS::DynamicData*) — example: get a member named "speed" if exists
      try {
        int32_t speed = 0;

        DDS::ReturnCode_t get_rc = data->get_int32_value(speed, /*member id or name*/ 1);
        // OR use member name APIs (depends on how you want to access)
        // Note: many get_*_value methods exist on DDS::DynamicData

      } catch (const CORBA::Exception& ex) {
        ACE_ERROR((LM_ERROR, "Exception reading dynamic data: %C\n", ex._info().c_str()));
      }

      // ... process numeric_data_info_, string_data_info_, callbacks etc
    }
    else if (rc == DDS::RETCODE_NO_DATA) {
      // no more data available
      break;
    }
  } // while
}


////////////////////////////////////////////////////
// VALUES METHODS
////////////////////////////////////////////////////

std::string ReaderHandler::topic_name() const
{
    return topic_->get_name();
}

const std::string& ReaderHandler::type_name() const
{
    return topic_->get_type_name();
}

////////////////////////////////////////////////////
// AUXILIAR METHODS
////////////////////////////////////////////////////

void ReaderHandler::create_data_structures_(
        DDS::DynamicData_ptr data /* = nullptr */)
{
    // Serialize data to JSON format
    nlohmann::json serialized_data;
    if (DDS::RETCODE_OK != utils::serialize_data(data, serialized_data))
    {
        DDS_ERROR("READER_HANDLER", "Error serializing data");
        return;
    }
    // Create the structures to store the data introspection information AND the data itself
    utils::get_formatted_data(
        topic_name(),
        data_type_configuration_,
        numeric_data_info_,
        string_data_info_,
        serialized_data);

    DEBUG("Completed type introspection created in topic: " << topic_name() << " with types: ");
    for (const auto& info : numeric_data_info_)
    {
        DEBUG("\tNumeric: " << std::get<0>(info));
    }
    for (const auto& info : string_data_info_)
    {
        DEBUG("\tString: " << std::get<0>(info));
    }
}

////////////////////////////////////////////////////
// AUXILIAR STATIC METHODS
////////////////////////////////////////////////////

DDS::StatusMask ReaderHandler::default_listener_mask_()
{
    // Only listen for DATA_AVAILABLE (plus participant-related statuses if needed)
    return DDS::DATA_AVAILABLE_STATUS;
}


std::vector<std::string> ReaderHandler::numeric_data_series_names() const
{
    return utils::get_introspection_type_names(numeric_data_info_);
}

std::vector<std::string> ReaderHandler::string_data_series_names() const
{
    return utils::get_introspection_type_names(string_data_info_);
}

} /* namespace fastdds */
} /* namespace plotjuggler */
} /* namespace eprosima */
