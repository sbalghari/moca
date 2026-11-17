#ifndef MOCA_CORE_HTTP_HPP
#define MOCA_CORE_HTTP_HPP

#include <cpr/cpr.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>

#include <string>

// HTTP requests

cpr::Response post_chat_completion(const std::string &base_url,
                                   const std::string &api_key,
                                   const nlohmann::json &request_body);

cpr::Response get_models(const std::string &base_url,
                         const std::string &api_key);

#endif // MOCA_CORE_HTTP_HPP