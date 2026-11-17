#include "cli.hpp"
#include "core/agent.hpp"
#include "core/config.hpp"
#include "utils/errors.hpp"

#include <filesystem>
#include <iostream>

int main(int argc, char *argv[]) {
    CLI cli;

    try {
        CLIOptions options = cli.parse_arguments(argc, argv);

        if (options.help) {
            cli.print_help();
            return 0;
        }

        if (options.version) {
            cli.print_version();
            return 0;
        }

        // --init just makes sure the file exists; it never touches an existing
        // one.
        if (options.init) {
            const auto path = MocaConfig::get_config_path();

            if (std::filesystem::exists(path)) {
                std::cout << "Config already exists: " << path.string() << '\n';
                return 0;
            }

            MocaConfig config;
            config.data() = MocaConfig::defaults();
            config.save();

            std::cout << "Created config file: " << path.string() << '\n';
            return 0;
        }

        MocaConfig config;
        config.load();

        if (!options.model.empty())
            config.set_model_override(options.model);

        if (!options.api_key.empty())
            config.set_api_key_override(options.api_key);

        if (options.list_models) {
            AgentMainloop::print_available_models(config);
            return 0;
        }

        // Start the agent.
        return AgentMainloop::start(config, options.prompt);
    } catch (const MocaBaseException &e) {
        std::cerr << "moca: " << e.what() << '\n';
        return 1;
    }
}
