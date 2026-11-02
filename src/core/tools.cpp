#include "tools.hpp"

#include "../utils/errors.hpp"

#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

const nlohmann::json kToolDefinitions = nlohmann::json::array(
    {{{"type", "function"},
      {"function",
       {{"name", "Read"},
        {"description", "Read and return the contents of a file"},
        {"parameters",
         {{"type", "object"},
          {"properties",
           {{"file_path",
             {{"type", "string"},
              {"description", "The path to the file to read"}}}}},
          {"required", nlohmann::json::array({"file_path"})}}}}}}});

std::string read_file(const std::string &path) {
    std::ifstream file(path);

    if (!file) {
        throw ToolException(
            std::format("Read Tool failed! failed to read file: '{}'", path));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

} // namespace

const nlohmann::json &tool_definitions() { return kToolDefinitions; }

int handle_tool_call(const nlohmann::json &tool_call) {
    std::string tool_name = tool_call["function"]["name"].get<std::string>();

    nlohmann::json arguments = nlohmann::json::parse(
        tool_call["function"]["arguments"].get<std::string>());

    try {
        if (tool_name == "Read") {
            std::string tool_path = arguments["file_path"].get<std::string>();

            std::cout << read_file(tool_path);
        }
    } catch (const ToolException &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}