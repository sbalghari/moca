#include "config.hpp"

#include "../utils/errors.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>

namespace {

// Provider presets live in code as a static data owned by the program instead
// of config.json, and/so the user just needs to tell which provider to use.
const nlohmann::json kProviders = {
    {"openrouter",
     {{"name", "OpenRouter"},
      {"default_model", "openrouter/free"},
      {"urls",
       {{"base", "https://openrouter.ai/api/v1"},
        {"chat_completion", "/chat/completions"}}},
      {"capabilities",
       {{"streaming", true},
        {"tool_calling", true},
        {"structured_outputs", true},
        {"model_dependent", true}}},
      {"documentation",
       {{"api", "https://openrouter.ai/docs/api-reference/overview"},
        {"models", "https://openrouter.ai/models"},
        {"quickstart", "https://openrouter.ai/docs/quickstart"}}}}}};

// Get the names of all the known/available provider's names from `kProviders`
// and return them as a single comma seperated string
std::string known_providers() {
    std::string names;

    for (auto it = kProviders.begin(); it != kProviders.end(); ++it) {
        if (!names.empty())
            names += ", ";

        names += it.key();
    }

    return names;
}

// Get a provider's full preset from its name
const nlohmann::json &provider_preset(const std::string &name) {
    const auto it = kProviders.find(name);

    if (it == kProviders.end())
        throw ConfigException(
            "Unknown provider '" + name +
            "' in config. Known providers: " + known_providers());

    return *it;
}

std::string string_value(const nlohmann::json &config, const std::string &key) {
    const auto it = config.find(key);

    if (it == config.end() || !it->is_string())
        return {};

    return it->get<std::string>();
}

std::string env_value(const char *name) {
    const char *value = std::getenv(name);

    if (!value)
        return {};

    return value;
}

} // namespace

MocaConfig::MocaConfig(std::filesystem::path path) : path_(std::move(path)) {}

void MocaConfig::load() {
    // Always start from the defaults. The file is then merged on top of them,
    // so an incompelete config.json still produces a complete configuration.
    config_ = defaults();

    if (!std::filesystem::exists(path_)) {
        // Most likely, first run: write the defaults out so the user has a file
        // to edit.
        save();
        return;
    }

    try {
        std::ifstream file(path_);

        if (!file.is_open())
            throw ConfigException("Could not open config file: " +
                                  path_.string());

        // update() merges recursively: keys from the file win, keys that only
        // exist in the defaults are kept, and unknown keys are preserved.
        config_.update(nlohmann::json::parse(file));
    } catch (const nlohmann::json::exception &e) {
        throw ConfigException("Invalid config file '" + path_.string() +
                              "': " + e.what());
    }

    // Validates the values. Doing it here gives a message that names the
    // bad key, instead of a confusing nlohmann type error later in the agent.
    for (const char *key : {"provider", "model", "api_key"}) {
        if (!config_.contains(key) || !config_[key].is_string())
            throw ConfigException("Config key '" + std::string(key) +
                                  "' must be a string in " + path_.string());
    }

    // This is a validation call that checks weather the configured provider
    // name is supported(present in kProviders) or not(not present in
    // kProviders). Throws for an unknown provider name
    provider_preset(provider());
}

void MocaConfig::save() const {
    std::error_code ec;

    const auto dir = path_.parent_path();

    if (!dir.empty())
        std::filesystem::create_directories(dir, ec);

    if (ec)
        throw ConfigException("Could not create config directory '" +
                              dir.string() + "': " + ec.message());

    // Write to a sibling temp file, then move it over the real file. rename()
    // is atomic on the same filesystem, so an interrupted save cannot destroy
    // an existing config.
    auto tmp_path = path_;
    tmp_path += ".tmp";

    {
        std::ofstream file(tmp_path);

        if (!file.is_open())
            throw ConfigException("Could not write config file: " +
                                  tmp_path.string());

        file << config_.dump(4) << '\n';

        if (!file)
            throw ConfigException("Failed while writing config file: " +
                                  tmp_path.string());
    }

    std::filesystem::rename(tmp_path, path_, ec);

    if (ec) {
        std::filesystem::remove(tmp_path);

        throw ConfigException("Could not replace config file '" +
                              path_.string() + "': " + ec.message());
    }

// TODO: Implement permissions on windows, its too complex for me rn :)
// see:
// https://learn.microsoft.com/en-us/windows/win32/secauthz/defining-permissions-with-acls-in-c--
#ifndef _WIN32
    // config.json may hold an API key, so make it readable by its owner only.
    // Best effort: a filesystem without permissions support (or an odd umask)
    // should not turn a successful save into an error.
    std::filesystem::permissions(path_,
                                 std::filesystem::perms::owner_read |
                                     std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::replace, ec);
#endif
}

nlohmann::json &MocaConfig::data() { return config_; }

const nlohmann::json &MocaConfig::data() const { return config_; }

std::string MocaConfig::provider() const {
    const auto name = string_value(config_, "provider");

    return name.empty() ? "openrouter" : name;
}

std::string MocaConfig::model() const {
    if (model_override_)
        return *model_override_;

    const auto from_file = string_value(config_, "model");

    if (!from_file.empty())
        return from_file;

    // Last resort: whatever the selected provider recommends.
    return provider_preset(provider())["default_model"].get<std::string>();
}

std::string MocaConfig::api_key() const {
    if (api_key_override_)
        return *api_key_override_;

    const auto from_env = env_value("MOCA_API_KEY");

    if (!from_env.empty())
        return from_env;

    return string_value(config_, "api_key");
}

std::string MocaConfig::base_url() const {
    // base_url is optional in the file and only exists so a user can point at
    // a proxy or a local endpoint. It is not written by defaults().
    const auto from_file = string_value(config_, "base_url");

    if (!from_file.empty())
        return from_file;

    return provider_preset(provider())["urls"]["base"].get<std::string>();
}

void MocaConfig::set_model_override(std::string model) {
    model_override_ = std::move(model);
}

void MocaConfig::set_api_key_override(std::string api_key) {
    api_key_override_ = std::move(api_key);
}

nlohmann::json MocaConfig::defaults() {
    return {
        {"version", 1},
        {"provider", "openrouter"},
        {"model", kProviders.at("openrouter")["default_model"]},
        {"api_key", ""},
    };
}

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

std::filesystem::path MocaConfig::get_config_path() {
    return get_config_dir() / "config.json";
}
