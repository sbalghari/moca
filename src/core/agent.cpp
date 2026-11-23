#include "agent.hpp"

#include "http.hpp"
#include "tools.hpp"

#include "../utils/errors.hpp"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>

using json = nlohmann::json;

namespace {

// Amount of model <-> tool round trips allowed.
constexpr int kMaxSteps = 20;

// The "messages" array is the whole conversation: the original prompt, every
// assistant reply, and every tool result. The model has no memory between
// requests, so we send this entire history every time.
json build_request_body(const json &messages, const std::string &model) {
    return {{"model", model},
            {"messages", messages},
            {"tools", tool_definitions()}};
}

std::string require_api_key(const MocaConfig &config) {
    std::string api_key = config.api_key();

    if (api_key.empty())
        throw ConfigException(
            "No API key configured. Export MOCA_API_KEY, pass -k <KEY>, or set "
            "\"api_key\" in " +
            MocaConfig::get_config_path().string());

    return api_key;
}

// Print one short line so the user can see the agent working. Only "command"
// and "file_path" are shown on purpose: Write's "content" can be a whole file
// and would flood the terminal.
void print_tool_activity(const json &tool_call) {
    std::cout << "[tool]";

    try {
        const json &function = tool_call.at("function");
        std::cout << ' ' << function.at("name").get<std::string>();

        const json arguments =
            json::parse(function.at("arguments").get<std::string>());

        if (arguments.contains("command"))
            std::cout << ": " << arguments["command"].get<std::string>();
        else if (arguments.contains("file_path"))
            std::cout << ": " << arguments["file_path"].get<std::string>();
    } catch (const json::exception &) {
        // Bad arguments are reported by execute_tool; nothing to show here.
    }

    std::cout << std::endl;
}

} // namespace

int AgentMainloop::start(const MocaConfig &config, const std::string &prompt) {
    const std::string api_key = require_api_key(config);

    // The conversation starts with the user's prompt.
    json messages = json::array({{{"role", "user"}, {"content", prompt}}});

    for (int step = 1; step <= kMaxSteps; ++step) { // The actual main loop
        const cpr::Response response =
            post_chat_completion(config.base_url(), api_key,
                                 build_request_body(messages, config.model()));

        if (response.status_code != 200) {
            std::cerr << "HTTP error: " << response.status_code << '\n'
                      << response.text << '\n';
            return 1;
        }

        json result;
        try {
            result = json::parse(response.text);
        } catch (const json::exception &e) {
            std::cerr << "Could not parse API response: " << e.what() << '\n';
            return 1;
        }

        if (!result.contains("choices") || result["choices"].empty()) {
            std::cerr << "No choices in response\n";
            return 1;
        }

        // Remember what the assistant said, including the tool calls it
        // asked for.
        const json message = result["choices"][0]["message"];
        messages.push_back(message);

        // No tool calls means the model is finished, so print the final message
        // and exit.
        if (!message.contains("tool_calls") || message["tool_calls"].empty()) {
            if (message.contains("content") && message["content"].is_string())
                std::cout << message["content"].get<std::string>() << std::endl;
            return 0;
        }

        // Run every requested tool and add each result to the conversation.
        for (const json &tool_call : message["tool_calls"]) {
            print_tool_activity(tool_call);

            const std::string tool_output = execute_tool(tool_call);

            // The model matches this result to its request using the id.
            messages.push_back({{"role", "tool"},
                                {"tool_call_id", tool_call.value("id", "")},
                                {"content", tool_output}});
        }
        // THE LOOP REPEATS...
    }

    std::cerr << "Stopped after " << kMaxSteps << " steps.\n";
    return 1;
}

void AgentMainloop::print_available_models(const MocaConfig &config) {
    const std::string api_key = require_api_key(config);

    cpr::Response response = get_models(config.base_url(), api_key);

    if (response.status_code != 200) {
        std::cerr << "Failed to fetch models: " << response.status_code << '\n'
                  << response.text << '\n';
        return;
    }

    json data = json::parse(response.text);

    for (const auto &model : data["data"]) {
        std::cout << model["id"].get<std::string>() << " - "
                  << model["name"].get<std::string>() << '\n';
    }
}
