#ifndef MOCA_CORE_CONFIG_HPP
#define MOCA_CORE_CONFIG_HPP

#include <string>

struct MocaConfig {
    std::string app_name = "moca";
    std::string app_ver = "0.1.0";
    std::string model = "openrouter/free";
    std::string api_key;
    std::string base_url = "https://openrouter.ai/api/v1";
    bool verbose = false;

    MocaConfig();
};

#endif // MOCA_CORE_CONFIG_HPP