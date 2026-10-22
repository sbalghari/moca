#include "core/agent.hpp"

#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 3 || std::string(argv[1]) != "-p") {
        std::cerr << "Expected first argument to be '-p'" << std::endl;
        return 1;
    }

    std::string prompt = argv[2];

    if (prompt.empty()) {
        std::cerr << "Prompt must not be empty" << std::endl;
        return 1;
    }

    // Start the agent.
    return AgentMainloop::start(prompt);
}