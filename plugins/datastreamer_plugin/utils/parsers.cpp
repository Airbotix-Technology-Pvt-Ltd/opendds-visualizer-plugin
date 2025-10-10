#include "parsers.hpp"

#include <iostream>

using json = nlohmann::json;

namespace airbotix {
namespace plotjuggler {
namespace opendds {

// Forward declarations
static json parse_dynamic_data_primitive(const DDS::DynamicData_var& data, DDS::MemberId id, OpenDDS::XTypes::TypeKind tk);
static json parse_dynamic_data_collection(const DDS::DynamicData_var& data);
static json parse_dynamic_data_aggregated(const DDS::DynamicData_var& data);
//------------------------------------------------------------------------------
// JSON parsing helpers for DynamicData and OpenDynamicData
//------------------------------------------------------------------------------

json parse_dynamic_data_primitive(const DDS::DynamicData_var& data, DDS::MemberId id, OpenDDS::XTypes::TypeKind tk) {
    DDS::ReturnCode_t ret;
    switch (tk) {
        case OpenDDS::XTypes::TK_INT32: {
            CORBA::Long val; ret = data->get_int32_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_UINT32: {
            CORBA::ULong val; ret = data->get_uint32_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_INT16: {
            CORBA::Short val; ret = data->get_int16_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_UINT16: {
            CORBA::UShort val; ret = data->get_uint16_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_INT64: {
            CORBA::LongLong val; ret = data->get_int64_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_UINT64: {
            CORBA::ULongLong val; ret = data->get_uint64_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_FLOAT32: {
            CORBA::Float val; ret = data->get_float32_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_FLOAT64: {
            CORBA::Double val; ret = data->get_float64_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_BOOLEAN: {
            CORBA::Boolean val; ret = data->get_boolean_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(static_cast<bool>(val)) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_BYTE: {
            CORBA::Octet val; ret = data->get_byte_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(val) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_CHAR8: {
            CORBA::Char val; ret = data->get_char8_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(std::string(1, val)) : json(nullptr);
        }
        case OpenDDS::XTypes::TK_STRING8: {
            CORBA::String_var val; ret = data->get_string_value(val, id);
            return (ret == DDS::RETCODE_OK) ? json(std::string(val.in())) : json(nullptr);
        }
        default:
            return json(nullptr);
    }
}

json parse_dynamic_data(const DDS::DynamicData_var& data);

json parse_dynamic_data_collection(const DDS::DynamicData_var& data) {
    json result = json::array();
    DDS::DynamicType_var type = data->type();
    DDS::TypeDescriptor_var td;
    if (type->get_descriptor(td) != DDS::RETCODE_OK) return result;

    const unsigned int count = data->get_item_count();
    DDS::DynamicType_var elem_type = OpenDDS::XTypes::get_base_type(td->element_type());
    const OpenDDS::XTypes::TypeKind elem_tk = elem_type->get_kind();

    for (unsigned int i = 0; i < count; ++i) {
        DDS::MemberId id = data->get_member_id_at_index(i);
        if (id == OpenDDS::XTypes::MEMBER_ID_INVALID) continue;

        switch (elem_tk) {
            case OpenDDS::XTypes::TK_SEQUENCE:
            case OpenDDS::XTypes::TK_ARRAY:
            case OpenDDS::XTypes::TK_STRUCTURE:
            case OpenDDS::XTypes::TK_UNION: {
                DDS::DynamicData_var nested_data;
                if (data->get_complex_value(nested_data, id) == DDS::RETCODE_OK)
                    result.push_back(parse_dynamic_data(nested_data));
                break;
            }
            default:
                result.push_back(parse_dynamic_data_primitive(data, id, elem_tk));
                break;
        }
    }
    return result;
}

json parse_dynamic_data_aggregated(const DDS::DynamicData_var& data) {
    json result = json::object();
    DDS::DynamicType_var type = data->type();
    const unsigned int count = data->get_item_count();

    for (unsigned int i = 0; i < count; ++i) {
        DDS::MemberId id = data->get_member_id_at_index(i);
        if (id == OpenDDS::XTypes::MEMBER_ID_INVALID) continue;

        DDS::DynamicTypeMember_var dtm;
        if (type->get_member(dtm, id) != DDS::RETCODE_OK) continue;

        DDS::MemberDescriptor_var md;
        if (dtm->get_descriptor(md) != DDS::RETCODE_OK) continue;

        std::string member_name = md->name();
        const DDS::DynamicType_var base_type = OpenDDS::XTypes::get_base_type(md->type());
        const OpenDDS::XTypes::TypeKind member_tk = base_type->get_kind();

        switch (member_tk) {
            case OpenDDS::XTypes::TK_SEQUENCE:
            case OpenDDS::XTypes::TK_ARRAY:
            case OpenDDS::XTypes::TK_STRUCTURE:
            case OpenDDS::XTypes::TK_UNION: {
                DDS::DynamicData_var nested_data;
                if (data->get_complex_value(nested_data, id) == DDS::RETCODE_OK)
                    result[member_name] = parse_dynamic_data(nested_data);
                break;
            }
            default:
                result[member_name] = parse_dynamic_data_primitive(data, id, member_tk);
                break;
        }
    }
    return result;
}

json parse_dynamic_data(const DDS::DynamicData_var& data) {
    const OpenDDS::XTypes::TypeKind tk = data->type()->get_kind();
    switch (tk) {
        case OpenDDS::XTypes::TK_SEQUENCE:
        case OpenDDS::XTypes::TK_ARRAY:
            return parse_dynamic_data_collection(data);
        case OpenDDS::XTypes::TK_STRUCTURE:
        case OpenDDS::XTypes::TK_UNION:
            return parse_dynamic_data_aggregated(data);
        default:
            return json(nullptr);
    }
}

json parse_dynamic_data(const std::shared_ptr<airbotix::plotjuggler::opendds::OpenDynamicData>& data) {
    json result = json::object();
    const size_t childCount = data->getLength();
    for (size_t i = 0; i < childCount; i++) {
        const std::shared_ptr<airbotix::plotjuggler::opendds::OpenDynamicData> child = data->getMember(i);
        if (!child) continue;

        std::string name = child->getFullName();

        if (child->isContainerType()) {
            result[name] = parse_dynamic_data(child);
            continue;
        }

        CORBA::TCKind kind = child->getKind();
        switch (kind) {
            case CORBA::tk_long: result[name] = child->getValue<int32_t>(); break;
            case CORBA::tk_short: result[name] = child->getValue<int16_t>(); break;
            case CORBA::tk_ushort: result[name] = child->getValue<uint16_t>(); break;
            case CORBA::tk_ulong: result[name] = child->getValue<uint32_t>(); break;
            case CORBA::tk_float: result[name] = child->getValue<float>(); break;
            case CORBA::tk_double: result[name] = child->getValue<double>(); break;
            case CORBA::tk_boolean: result[name] = static_cast<bool>(child->getValue<uint32_t>()); break;
            case CORBA::tk_char: result[name] = std::string(1, child->getValue<char>()); break;
            case CORBA::tk_octet: result[name] = child->getValue<uint8_t>(); break;
            case CORBA::tk_longlong: result[name] = child->getValue<int64_t>(); break;
            case CORBA::tk_ulonglong: result[name] = child->getValue<uint64_t>(); break;
            case CORBA::tk_string: result[name] = std::string(child->getStringValue()); break;
            case CORBA::tk_enum: {
                const CORBA::ULong enumValue = child->getValue<CORBA::ULong>();
                CORBA::TypeCode_var enumTypeCode = child->getTypeCode();
                const CORBA::ULong enumMemberCount = enumTypeCode->member_count();
                if (enumValue < enumMemberCount) {
                    result[name] = std::string(enumTypeCode->member_name(enumValue));
                } else {
                    result[name] = "INVALID";
                }
                break;
            }
            default:
                result[name] = nullptr;
                break;
        }
    }
    return result;
}

} // namespace opendds
} // namespace plotjuggler
} // namespace airbotix