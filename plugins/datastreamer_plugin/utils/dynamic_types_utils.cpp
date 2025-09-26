#include "dynamic_types_utils.hpp"
#include "utils/Exception.hpp"
#include "utils.hpp"
#include <dds/DCPS/JsonValueWriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <dds/DCPS/XTypes/DynamicVwrite.h>  // For vwrite DynamicData overload
#include <dds/DCPS/JsonValueWriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <limits>

namespace eprosima {
namespace plotjuggler {
namespace utils {

template std::vector<std::string> get_introspection_type_names<TypeIntrospectionNumericStruct>(
        const TypeIntrospectionNumericStruct& type_names_struct);
template std::vector<std::string> get_introspection_type_names<TypeIntrospectionStringStruct>(
        const TypeIntrospectionStringStruct& type_names_struct);

template <typename T>
std::vector<std::string> get_introspection_type_names(
        const T& type_names_struct)
{
    std::vector<std::string> type_names;
    for (const auto& type_name : type_names_struct)
    {
        type_names.push_back(type_name.first);
    }
    return type_names;
}

void get_formatted_data(
        const std::string& base_type_name,
        const DataTypeConfiguration& data_type_configuration,
        TypeIntrospectionNumericStruct& numeric_data,
        TypeIntrospectionStringStruct& string_data,
        const nlohmann::json& data,
        const std::string& separator /* = "/" */)
{
    if (is_kind_numeric(data))
    {
        numeric_data.push_back({base_type_name, data.get<double>()});
        return;
    }
    else if (is_kind_boolean(data))
    {
        numeric_data.push_back({base_type_name, static_cast<double>(data.get<bool>())});
        return;
    }
    else if (is_kind_string(data))
    {
        string_data.push_back({base_type_name, data.get<std::string>()});
        return;
    }
    else if (data.is_null())
    {
        return;
    }
    else if (data.is_array())
    {
        if (data.size() >= data_type_configuration.max_array_size)
        {
            if (data_type_configuration.discard_large_arrays)
            {
                DEBUG("Discarding array " << base_type_name << " of size " << data.size());
                return;
            }
            else
            {
                DEBUG("Truncating array " << base_type_name << " of size " << data.size() 
                      << " to size " << data_type_configuration.max_array_size);
            }
        }

        for (int i = 0; i < std::min(static_cast<unsigned>(data.size()), data_type_configuration.max_array_size); i++)
        {
            get_formatted_data(base_type_name + "[" + std::to_string(i) + "]",
                               data_type_configuration,
                               numeric_data,
                               string_data,
                               data[i],
                               separator);
        }
    }
    else if (data.is_object())
    {
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            get_formatted_data(base_type_name + separator + it.key(),
                               data_type_configuration,
                               numeric_data,
                               string_data,
                               it.value(),
                               separator);
        }
    }
    else
    {
        std::cerr << "Data type not supported in get_formatted_data for key: " 
                  << base_type_name << std::endl;
        return;
    }
}

bool is_kind_numeric(const nlohmann::json& data)
{
    return data.is_number();
}

bool is_kind_boolean(const nlohmann::json& data)
{
    return data.is_boolean();
}

bool is_kind_string(const nlohmann::json& data)
{
    return data.is_string();
}






DDS::ReturnCode_t serialize_data(
    DDS::DynamicData_ptr data,
    nlohmann::json& serialized_data)
{
    // Check for null data
    if (!data)
    {
        std::cerr << "DYNAMIC_TYPES_UTILS: Data is nullptr. Skipping serialization to JSON format." << std::endl;
        return DDS::RETCODE_NO_DATA;
    }

    // Validate DynamicType
    DDS::DynamicType_ptr type = data->type();
    if (!type)
    {
        std::cerr << "DYNAMIC_TYPES_UTILS: DynamicData has no associated DynamicType." << std::endl;
        return DDS::RETCODE_ERROR;
    }

    // Create RapidJSON StringBuffer and Writer
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    // Serialize using to_json, which calls vwrite internally
    DDS::ReturnCode_t retcode = OpenDDS::DCPS::to_json(data, writer);
    if (retcode != DDS::RETCODE_OK)
    {
        std::cerr << "DYNAMIC_TYPES_UTILS: Error encountered while serializing DynamicData to JSON: " << retcode << std::endl;
        return retcode;
    }

    // Parse the JSON string into nlohmann::json
    try
    {
        serialized_data = nlohmann::json::parse(buffer.GetString());
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cerr << "DYNAMIC_TYPES_UTILS: Failed to parse JSON string: " << e.what() << std::endl;
        return DDS::RETCODE_ERROR;
    }

    return DDS::RETCODE_OK;
}

} /* namespace utils */
} /* namespace plotjuggler */
} /* namespace eprosima */
