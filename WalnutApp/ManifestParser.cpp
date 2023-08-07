#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "ManifestParser.h"

// Helper function to skip whitespace characters
void skipWhitespace(std::istringstream& input) {
    while (std::isspace(input.peek())) {
        input.get();
    }
}

// Helper function to parse strings enclosed in double quotes
JsonValue parseString(std::istringstream& input) {
    char c;
    std::string value;
    input.get(); // Consume the opening double quote

    while (input.get(c)) {
        if (c == '"') {
            break;
        }
        value += c;
    }

    JsonValue jsonValue;
    jsonValue.value = value;
    return jsonValue;
}

// Function to parse a JSON object
JsonValue parseObject(std::istringstream& input) {
    JsonValue obj;
    std::string key;

    while (input.peek() != '}') {
        skipWhitespace(input);
        key = parseString(input).value; // Extract the value from the JsonValue object
        skipWhitespace(input);

        if (input.peek() == '{') {
            input.get(); // Consume the opening curly brace
            obj.object[key] = parseObject(input);
            skipWhitespace(input);
        }
        else if (input.peek() == '"') {
            obj.object[key] = parseString(input);
        }

        if (input.peek() == '}') {
            break;
        }

        // Consume the comma if there are more key-value pairs
        input.get();
    }

    input.get(); // Consume the closing curly brace
    return obj;
}
namespace m_parser
{
    // Function to parse the top-level JSON object
    JsonValue parseJson(std::istringstream& input) {
        skipWhitespace(input);
        return parseObject(input);
    }

    // Function to convert the parsed JSON object back to a JSON string
    std::string toJsonString(const JsonValue& value) {
        std::string jsonStr;

        if (!value.object.empty()) {
            jsonStr += "{";
            for (const auto& kv : value.object) {
                jsonStr += "\"" + kv.first + "\":";
                jsonStr += toJsonString(kv.second) + ",";
            }
            if (jsonStr.back() == ',') {
                jsonStr.pop_back();
            }
            jsonStr += "}";
        }
        else if (!value.array.empty()) {
            jsonStr += "{";
            for (const auto& item : value.array) {
                jsonStr += toJsonString(item) + ",";
            }
            if (jsonStr.back() == ',') {
                jsonStr.pop_back();
            }
            jsonStr += "}";
        }
        else {
            jsonStr += "\"" + value.value + "\"";
        }

        return jsonStr;
    }
}