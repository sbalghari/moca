#include "config.hpp"

#include "../utils/errors.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

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

nlohmann::json &MocaConfig::data() { return config_; }

const nlohmann::json &MocaConfig::data() const { return config_; }

std::filesystem::path MocaConfig::get_config_dir() {
#ifdef _WIN32
    const char *appdata = std::getenv("APPDATA");

    if (!appdata)
        throw ConfigException("Enviroment variable 'APPDATA' is not set");

    return std::filesystem::path(appdata) / "Moca";

#else
    const char *xdg_config = std::getenv("XDG_CONFIG_HOME");

    // Check whether xdg_config is non-null and contains a meaningful value,
    // rather than an empty string or a null terminator.
    if (xdg_config && *xdg_config)
        return std::filesystem::path(xdg_config) / "moca";

    const char *home = std::getenv("HOME");

    if (!home)
        throw ConfigException("Enviroment variable 'HOME' is not set!");

    return std::filesystem::path(home) / ".config" / "moca";
#endif
}