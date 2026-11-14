#ifndef MOCA_CLI_HPP
#define MOCA_CLI_HPP

#include <string>

struct CLIOptions {
    std::string prompt;
    std::string model;
    std::string api_key;

    bool help = false;
    bool version = false;
    bool list_models = false;
};

class CLI {
  public:
    static CLIOptions parse_arguments(int argc, char *argv[]);

    static void print_help();
    static void print_version();
};

#endif // MOCA_CLI_HPP