#ifndef MOCA_CORE_TOOLS_HPP
#define MOCA_CORE_TOOLS_HPP

#include <nlohmann/json.hpp>

const nlohmann::json &tool_definitions();

int handle_tool_call(const nlohmann::json &tool_call);

#endif // MOCA_CORE_TOOLS_HPP