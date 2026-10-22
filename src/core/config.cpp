#include "config.hpp"

#include "../utils/errors.hpp"

#include <cstdlib>
#include <format>
#include <string>

namespace {

std::string get_env(const std::string& key) {
    const char* value = std::getenv(key.c_str());

    if (value != nullptr) {
        return value;
    }

    throw ConfigException(
        std::format("Environment variable '{}' not defined!", key));
}

} // namespace

MocaConfig::MocaConfig()
    : api_key(get_env("MOCA_API_KEY")) {}