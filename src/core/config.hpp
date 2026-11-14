#ifndef MOCA_CORE_CONFIG_HPP
#define MOCA_CORE_CONFIG_HPP

#include <filesystem>
#include <nlohmann/json.hpp>

// Providers
const nlohmann::json OpenRouter = {
    {"name", "OpenRouter"},
    {"default_model", "openrouter/free"},
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

    nlohmann::json& data();
    const nlohmann::json& data() const;

    static std::filesystem::path get_config_dir();

private:
    std::filesystem::path path_;
    nlohmann::json config_;
};

#endif // MOCA_CORE_CONFIG_HPP