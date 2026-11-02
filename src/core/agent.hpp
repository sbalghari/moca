#ifndef MOCA_CORE_AGENT_HPP
#define MOCA_CORE_AGENT_HPP

#include <string>

class AgentMainloop {
public:
    static int start(const std::string& prompt);
    static void print_available_models();
};

#endif // MOCA_CORE_AGENT_HPP