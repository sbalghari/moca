#include "cli.hpp"
#include "utils/errors.hpp"

#include <iostream>

CLIOptions CLI::parse_arguments(int argc, char *argv[]) {
    CLIOptions options;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            options.help = true;
        } else if (arg == "-v" || arg == "--version") {
            options.version = true;
        } else if (arg == "-L" || arg == "--list-models") {
            options.list_models = true;
        } else if (arg == "-p" || arg == "--prompt") {
            if (i + 1 >= argc) {
                throw CLIException("Missing value for " + arg);
            }

            options.prompt = argv[++i];

            if (options.prompt.empty()) {
                throw CLIException("Prompt must not be empty");
            }
        } else if (arg == "-m" || arg == "--model") {
            if (i + 1 >= argc) {
                throw CLIException("Missing value for " + arg);
            }

            options.model = argv[++i];

            if (options.model.empty()) {
                throw CLIException("Model must not be empty");
            }
        } else if (arg == "-k" || arg == "--api-key") {
            if (i + 1 >= argc) {
                throw CLIException("Missing value for " + arg);
            }

            options.api_key = argv[++i];

            if (options.api_key.empty()) {
                throw CLIException("API-KEY must not be empty");
            }
        } else {
            throw CLIException("Unknown option: " + arg);
        }
    }

    // Require a prompt unless executing a standalone command.
    if (!options.help && !options.version && !options.list_models &&
        options.prompt.empty()) {
        throw CLIException("A prompt is required. Use -h for help.");
    }

    return options;
}

void CLI::print_help() {
    std::cout << R"(
Moca - A Simple Coding Agent written in modern C++. Only OpenRouter is supported
as a model provider right now.

Usage:
  moca [OPTIONS]

Options:
  -p, --prompt <TEXT>     Prompt for the agent to start a session with
  -m, --model <MODEL>     Select a model for the session
  -k, --api-key <KEY>     Pass an API key explicitly for this session
  -L, --list-models       List available models
  -h, --help              Print this help message and exit
  -v, --version           Print the app's version and exit

)";
}

#ifndef MOCA_VERSION
#define MOCA_VERSION "unknown"
#endif

void CLI::print_version() { std::cout << MOCA_VERSION << '\n'; }