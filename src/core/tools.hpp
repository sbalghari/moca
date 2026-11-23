#ifndef MOCA_CORE_TOOLS_HPP
#define MOCA_CORE_TOOLS_HPP

#include <nlohmann/json.hpp>

#include <string>

// Returns the JSON list that tells the model which tools exists,
// and it is sent with every API request.
const nlohmann::json &tool_definitions();

// Runs one tool call returned by the model and returns its output as text.
//
// The returned string is what gets sent back to the model in a "tool" message,
// so failures are *returned* as text ("Error: ...") instead of thrown. That
// way the model can read the error and try a different approach.
std::string execute_tool(const nlohmann::json &tool_call);

#endif // MOCA_CORE_TOOLS_HPP
