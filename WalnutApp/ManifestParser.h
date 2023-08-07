#pragma once

// Define a data structure to represent JSON objects and arrays
struct JsonValue {
    std::unordered_map<std::string, JsonValue> object;
    std::vector<JsonValue> array;
    std::string value;
};

namespace m_parser
{
	JsonValue parseJson(std::istringstream& input);

    std::string toJsonString(const JsonValue& value);
}