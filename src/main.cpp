#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

json Tools =
    json::array({{{"type", "function"},
                  {"function",
                   {{"name", "Read"},
                    {"description", "Read and return the contents of a file"},
                    {"parameters",
                     {{"type", "object"},
                      {"properties",
                       {{"file_path",
                         {{"type", "string"},
                          {"description", "The path to the file to read"}}}}},
                      {"required", json::array({"file_path"})}}}}}}});

class MocaBaseException : public std::runtime_error {
  public:
    MocaBaseException(const std::string &msg) : std::runtime_error(msg) {}
};

class APIException : public MocaBaseException {
  public:
    using MocaBaseException::MocaBaseException;
};

class ToolException : public MocaBaseException {
  public:
    using MocaBaseException::MocaBaseException;
};

class ConfigException : public MocaBaseException {
  public:
    using MocaBaseException::MocaBaseException;
};

std::string get_env(const std::string &key) {
    const char *value = std::getenv(key.c_str());

    if (value != nullptr) {
        return value;
    }

    throw ConfigException(
        std::format("Environment variable '{}' not defined!", key));
}

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

// Basic configuration of the application with their default values
// We will update this from the cli's args or the config file in future
struct MOCA_CONFIG {
    std::string app_name = "moca";
    std::string app_ver = "0.1.0";
    std::string model = "openrouter/free";
    std::string api_key = get_env("MOCA_API_KEY");
    std::string base_url = "https://openrouter.ai/api/v1";
    bool verbose = false;
};

class AgentMainloop {
    static int handle_tool_calls(const json &tool_call) {
        std::string tool_name =
            tool_call["function"]["name"].get<std::string>();

        json arguments =
            json::parse(tool_call["function"]["arguments"].get<std::string>());

        try {
            if (tool_name == "Read") {
                std::string tool_path =
                    arguments["file_path"].get<std::string>();
                std::cout << read_file(tool_path);
            }
        } catch (const ToolException &e) {
            std::cerr << e.what() << '\n';
            return 1;
        }
        return 0;
    }

    static json build_request_body(const std::string &prompt,
                                   const MOCA_CONFIG &config) {
        return {{"model", config.model},
                {
                    "messages",
                    json::array({{{"role", "user"}, {"content", prompt}}}),
                },
                {"tools", Tools}};
    }

  public:
    static int start(const std::string &prompt) {
        MOCA_CONFIG config;
        json request_body = build_request_body(prompt, config);

        cpr::Response response =
            cpr::Post(cpr::Url{config.base_url + "/chat/completions"},
                      cpr::Header{{"Authorization", "Bearer " + config.api_key},
                                  {"Content-Type", "application/json"}},
                      cpr::Body{request_body.dump()});

        if (response.status_code != 200) {
            std::cerr << "HTTP error: " << response.status_code << std::endl;
            return 1;
        }

        json result = json::parse(response.text);

        if (!result.contains("choices") || result["choices"].empty()) {
            std::cerr << "No choices in response" << std::endl;
            return 1;
        }

        json message = result["choices"][0]["message"];

        if (!message.contains("tool_calls") || message["tool_calls"].empty()) {
            std::cerr << "No tools were called!";
            return 0;
        }
        json tool_call = message["tool_calls"][0];

        return handle_tool_calls(tool_call);
    }
};

int main(int argc, char *argv[]) {
    if (argc < 3 || std::string(argv[1]) != "-p") {
        std::cerr << "Expected first argument to be '-p'" << std::endl;
        return 1;
    }

    std::string prompt = argv[2];

    if (prompt.empty()) {
        std::cerr << "Prompt must not be empty" << std::endl;
        return 1;
    }

    // Start the main agent loop
    return AgentMainloop::start(prompt);
}
