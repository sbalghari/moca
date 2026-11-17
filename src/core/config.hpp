#ifndef MOCA_CORE_CONFIG_HPP
#define MOCA_CORE_CONFIG_HPP

#include <filesystem>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>

// Moca's main configuration manger, this class handles all config related
// things, this is intialized at the begining and used, after, througout the
// project and the rest of the program doesn't need to worry about configs after
class MocaConfig {
    std::filesystem::path path_;
    nlohmann::json config_;

    std::optional<std::string> model_override_;
    std::optional<std::string> api_key_override_;

  public:
    // The optional path exists so tests can use a temp config path rather then
    // the main config.json, on normal runs its automatically set by
    // 'get_config_dir'.
    explicit MocaConfig(std::filesystem::path path = get_config_path());

    // Reads config.json and merges it on top of the built-in defaults. When no
    // file exists yet it is created with the defaults.
    void load();

    // Writes config_ back to disk atomically. Creates the directory if needed.
    void save() const;

    // Config precedence:
    // 1. built-in defaults -- lowest priority
    // 2. config.json
    // 3. cli -- highes priority.
    // These funtions returns the values after applying the above precedence
    std::string provider() const;
    std::string model() const;
    std::string api_key() const;
    std::string base_url() const;

    // Values from the command line apply to this run only. They are stored
    // separately from config_ on purpose because they aren't meant to presist
    // and 'save' must not presist it.
    void set_model_override(std::string model);
    void set_api_key_override(std::string api_key);

    // Returns a mutable refrence to the private data member 'config_'.
    nlohmann::json &data();

    // Returns an imutable refrence to the private data member 'config_'.
    const nlohmann::json &data() const;

    static std::filesystem::path get_config_dir();
    static std::filesystem::path get_config_path();

    // The built-in defaults of Moca
    static nlohmann::json defaults();
};

#endif // MOCA_CORE_CONFIG_HPP
