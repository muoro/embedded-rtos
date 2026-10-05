#include "app/gateway_application.hpp"
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {

        const auto options = GatewayOptions::from_arguments(argc, argv);
        Logger logger(options.log_level, options.log_stderr);
        try {
            GatewayApplication application(options, logger);
            application.run();
        } catch (const std::exception& error) {
            logger.write(LogLevel::error, error.what());
            if (!options.log_stderr) std::cerr << error.what() << std::endl;
            return 1;
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
