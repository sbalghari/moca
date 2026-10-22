#ifndef MOCA_CORE_HTTP_HPP
#define MOCA_CORE_HTTP_HPP

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <string>

cpr::Response post_chat_completion(
    const std::string& base_url,
    const std::string& api_key,
    const nlohmann::json& request_body);

#endif // MOCA_CORE_HTTP_HPP