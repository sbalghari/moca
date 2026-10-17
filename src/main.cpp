#include <cpr/cpr.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

std::string read_file(const std::string &path) {
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

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

    const char *api_key_env = std::getenv("MOCA_API_KEY");
    const char *base_url_env = std::getenv("OPENROUTER_BASE_URL");

    std::string api_key = api_key_env ? api_key_env : "";
    std::string base_url =
        base_url_env ? base_url_env : "https://openrouter.ai/api/v1";

    if (api_key.empty()) {
        std::cerr << "OPENROUTER_API_KEY is not set" << std::endl;
        return 1;
    }

    json request_body = {
        {"model", "openrouter/free"},
        {
            "messages",
            json::array({{{"role", "user"}, {"content", prompt}}}),
        },
        {"tools",
         json::array(
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
                   {"required", json::array({"file_path"})}}}}}}})}};

    cpr::Response response =
        cpr::Post(cpr::Url{base_url + "/chat/completions"},
                  cpr::Header{{"Authorization", "Bearer " + api_key},
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
    } else {
        json tool_call = message["tool_calls"][0];
        std::string tool_name =
            tool_call["function"]["name"].get<std::string>();

        json arguments = json::parse(tool_call["function"]["arguments"].get<std::string>());

        if (tool_name == "Read") {
            std::string tool_path = arguments["file_path"].get<std::string>();
            std::cout << read_file(tool_path);
        }
    }

    // std::cout <<
    // result["choices"][0]["message"]["content"].get<std::string>();

    return 0;
}
