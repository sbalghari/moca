#include "config.hpp"

#include "../utils/errors.hpp"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <fstream>

namespace {
namespace fs = std::filesystem;

std::string get_env(const std::string &key) {
    const char *value = std::getenv(key.c_str());

    if (value != nullptr) {
        return value;
    }

    throw ConfigException(
        std::format("Environment variable '{}' not defined!", key));
}

} // namespace

void MocaConfig::load() {
    std::ifstream file(path_);

    if (!file.is_open()) {
        throw ConfigException("Could not open config file");
    }

    file >> config_;
}

void MocaConfig::save() const {
    std::ofstream file(path_);

    if (!file.is_open()) {
        throw ConfigException("Could not save config file");
    }

    file << config_.dump(4) << '\n';
}

nlohmann::json& MocaConfig::data() {
    return config_;
}

const nlohmann::json& MocaConfig::data() const {
    return config_;
}

fs::path MocaConfig::get_config_dir() {
#ifdef _WIN32
    const char *appdata = std::getenv("APPDATA");

    if (!appdata)
        throw std::runtime_error("APPDATA not set");

    return fs::path(appdata) / "Moca";

#else
    const char *xdg_config = std::getenv("XDG_CONFIG_HOME");

    // The first checks weather the xdg_config is a nullptr, and the second checks that xdg_config
    // have an actuall value and isn't just a empty string or \0
    if (xdg_config && *xdg_config)
        return fs::path(xdg_config) / "moca";

    const char *home = std::getenv("HOME");

    if (!home)
        throw std::runtime_error("HOME not set");

    return fs::path(home) / ".config" / "moca";
#endif
}