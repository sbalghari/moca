#include "cli/cli.hpp"
#include "core/agent.hpp"

int main(int argc, char *argv[]) {

    CLI cli;
    CLIOptions options = cli.parse_arguments(argc, argv);

    if (options.help) {
        cli.print_help();
        return 0;
    }

    if (options.version) {
        cli.print_version();
        return 0;
    }

    if (options.list_models) {
        AgentMainloop::print_available_models();
        return 0;
    }

    // Start the agent.
    return AgentMainloop::start(options.prompt);
}