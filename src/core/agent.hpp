#ifndef MOCA_CORE_AGENT_HPP
#define MOCA_CORE_AGENT_HPP

#include "config.hpp"

#include <string>

class AgentMainloop {
  public:
    // config is passed in by main so every command shares one loaded config.
    static int start(const MocaConfig &config, const std::string &prompt);
    static void print_available_models(const MocaConfig &config);
};

#endif // MOCA_CORE_AGENT_HPP
