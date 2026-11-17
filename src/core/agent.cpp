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

json build_request_body(const std::string &prompt, const std::string &model) {

    return {
        {"model", model},
        {"messages", json::array({{{"role", "user"}, {"content", prompt}}})},
        {"tools", tool_definitions()}};
}

// Both agent commands need a key. Failing here with one clear message beats
// sending an empty "Bearer " header and getting an opaque 401 back.
std::string require_api_key(const MocaConfig &config) {
    std::string api_key = config.api_key();

    if (api_key.empty())
        throw ConfigException(
            "No API key configured. Export MOCA_API_KEY, pass -k <KEY>, or set "
            "\"api_key\" in " +
            MocaConfig::get_config_path().string());

    return api_key;
}

} // namespace

int AgentMainloop::start(const MocaConfig &config, const std::string &prompt) {
    const std::string api_key = require_api_key(config);

    json request_body = build_request_body(prompt, config.model());

    cpr::Response response =
        post_chat_completion(config.base_url(), api_key, request_body);

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

    return handle_tool_call(tool_call);
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
