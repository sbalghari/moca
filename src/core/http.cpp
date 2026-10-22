#include "http.hpp"

cpr::Response post_chat_completion(
    const std::string& base_url,
    const std::string& api_key,
    const nlohmann::json& request_body) {

    return cpr::Post(
        cpr::Url{base_url + "/chat/completions"},
        cpr::Header{
            {"Authorization", "Bearer " + api_key},
            {"Content-Type", "application/json"}
        },
        cpr::Body{request_body.dump()});
}