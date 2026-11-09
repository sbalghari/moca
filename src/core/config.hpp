#ifndef MOCA_CORE_CONFIG_HPP
#define MOCA_CORE_CONFIG_HPP

#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Providers
const json OpenRouter = {
    {"name", "OpenRouter"},
    {"default_model", "openrouter/free"},
    {"api_key", ""},
    {"urls",
     {{"base", "https://openrouter.ai/api/v1"},
      {"chat_completion", "/chat/completions"},
      {"models", "/models"}}},
    {"capabilities",
     {{"streaming", true},
      {"tool_calling", true},
      {"structured_outputs", true},
      {"model_dependent", true}}},
    {"documentation",
     {{"api", "https://openrouter.ai/docs/api-reference/overview"},
      {"models", "https://openrouter.ai/models"},
      {"quickstart", "https://openrouter.ai/docs/quickstart"}}}};

class MocaConfig {
public:
    MocaConfig() : path_(get_config_dir()) {}

    void load();
    void save() const;

    json& data();
    const json& data() const;

    static std::filesystem::path get_config_dir();

private:
    std::filesystem::path path_;
    json config_;
};

#endif // MOCA_CORE_CONFIG_HPP