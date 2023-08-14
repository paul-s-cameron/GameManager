// #############################################
// # Author: scameronpaul                      #
// # File: ManifestParser.hpp                  #
// # Description: Parses Steam manifest files  #
// #              and returns a json object    #
// #############################################

#pragma once
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "json.hpp"

using json = nlohmann::json;

namespace m_parser
{
    inline void skipWhitespace(std::istringstream& input) {
        while (isspace(input.peek())) {
            input.get();
        }
    }

    inline std::string parseString(std::istringstream& input) {
        char c;
        std::string value;
        input.get(); // Consume the opening double quote

        while (input.get(c)) {
            if (c == '"') {
                break;
            }
            value += c;
        }

        return value;
    }

    inline json parseObject(std::istringstream& input) {
        json obj;
        std::string key;

        while (input.peek() != '}') {
            skipWhitespace(input);
            if (input.peek() == '}') // Check for empty object
                return {};
            key = parseString(input); // Extract the key value
            skipWhitespace(input);

            if (input.peek() == '{') {
                input.get(); // Consume the opening curly brace
                obj[key] = parseObject(input);
                skipWhitespace(input);
            }
            else if (input.peek() == '"') {
                obj[key] = parseString(input);
                skipWhitespace(input);
            }

            if (input.peek() == '}') {
                input.get(); // Consume the closing curly brace
                break;
            }
        }
        return obj;
    }

    inline json parseJson(std::istringstream& input) {
        // Skip first line
        std::string line;
        std::getline(input, line);

        // Consume the opening curly brace and check for empty object
        while (input.peek() != '{')
        {
            // Avoid infinite loop if file is empty
            if (input.peek() == EOF)
				return json::object();
            input.get();
        } input.get();

        json data;
        data = parseObject(input);

        return data;
    }
}