#include "tools.hpp"

#include "../utils/errors.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// popen/pclose start a command and read its output
#ifdef _WIN32
#define MOCA_POPEN _popen
#define MOCA_PCLOSE _pclose
#else
#define MOCA_POPEN popen
#define MOCA_PCLOSE pclose
#endif

namespace {

// Max tool output size, limited becase it is sent back to the model,
// so a huge output may eat tokens
constexpr std::size_t kMaxOutputChars = 8000;

const nlohmann::json kToolDefinitions = nlohmann::json::array({
    // Read
    {{"type", "function"},
     {"function",
      {{"name", "Read"},
       {"description", "Read and return the contents of a file"},
       {"parameters",
        {{"type", "object"},
         {"properties",
          {{"file_path",
            {{"type", "string"},
             {"description", "The path to the file to read"}}}}},
         {"required", nlohmann::json::array({"file_path"})}}}}}},

    // Write
    {{"type", "function"},
     {"function",
      {{"name", "Write"},
       {"description", "Write content to a file"},
       {"parameters",
        {{"type", "object"},
         {"properties",
          {{"file_path",
            {{"type", "string"},
             {"description", "The path of the file to write to"}}},
           {"content",
            {{"type", "string"},
             {"description", "The content to write to the file"}}}}},
         {"required", nlohmann::json::array({"file_path", "content"})}}}}}},

    // Bash
    {{"type", "function"},
     {"function",
      {{"name", "Bash"},
       {"description", "Execute a shell command"},
       {"parameters",
        {{"type", "object"},
         {"properties",
          {{"command",
            {{"type", "string"}, {"description", "The command to execute"}}}}},
         {"required", nlohmann::json::array({"command"})}}}}}},
});

std::string read_file(const std::string &path) {
    std::ifstream file(path);

    if (!file)
        throw ToolException("Read tool: could not open '" + path + "'");

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

std::string write_file(const std::string &path, const std::string &content) {
    const std::filesystem::path file_path(path);

    // Ensure parent dirs of 'path' exists
    if (file_path.has_parent_path())
        std::filesystem::create_directories(file_path.parent_path());

    // std::ios::trunc makes sure old content is replaced, not appended to.
    std::ofstream file(file_path, std::ios::trunc);

    if (!file)
        throw ToolException("Write tool: could not open '" + path + "'");

    file << content;

    // Check again: a write can fail halfway
    if (!file)
        throw ToolException("Write tool: failed while writing '" + path + "'");

    return "Wrote " + std::to_string(content.size()) + " bytes to " + path;
}

std::string run_bash(const std::string &command) {
    // popen starts a shell and gives us a pipe to read its output. Adding
    // "2>&1" sends error messages into that same pipe, so we capture them too.
    FILE *pipe = MOCA_POPEN((command + " 2>&1").c_str(), "r");

    if (!pipe)
        throw ToolException("Bash tool: could not start a shell");

    std::string output;
    char buffer[256];

    // fgets reads one line at a time (up to 255 characters).
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Let the command finish but stop collecting texts
        if (output.size() < kMaxOutputChars)
            output += buffer;
    }

    // pclose waits for the command to finish and returns its status.
    const int status = MOCA_PCLOSE(pipe);

    // Cut off anything the loop above went slightly past the limit.
    if (output.size() > kMaxOutputChars) {
        output.resize(kMaxOutputChars);
        output += "\n... (output truncated)";
    }

    if (output.empty())
        output = "(no output)";

    if (status != 0)
        output += "\n(command exited with a non-zero status)";

    return output;
}

} // namespace

const nlohmann::json &tool_definitions() { return kToolDefinitions; }

std::string execute_tool(const nlohmann::json &tool_call) {
    try {
        // A tool call looks like:
        // { "id": "...", "function": { "name": "Read", "arguments": "{...}" } }
        const nlohmann::json &function = tool_call.at("function");
        const std::string name = function.at("name").get<std::string>();

        // The arguments arrive as a JSON-encoded *string*, so they need a
        // second parse before we can read them like a normal object.
        const nlohmann::json arguments =
            nlohmann::json::parse(function.at("arguments").get<std::string>());

        // Pick the function that matches the name the model asked for.
        if (name == "Read")
            return read_file(arguments.at("file_path").get<std::string>());

        if (name == "Write")
            return write_file(arguments.at("file_path").get<std::string>(),
                              arguments.at("content").get<std::string>());

        if (name == "Bash")
            return run_bash(arguments.at("command").get<std::string>());

        return "Error: unknown tool '" + name + "'";
    } catch (const std::exception &e) {
        // Every failure becomes readable text for the model. std::exception
        // covers ToolException, JSON errors and filesystem errors alike.
        return "Error: " + std::string(e.what());
    }
}
