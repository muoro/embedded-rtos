#pragma once
#include <string>

enum class LogLevel { error, warning, info, debug };

// One process-wide syslog identity; owned by main for the application's lifetime.
class Logger {
  public:
    Logger(LogLevel level, bool mirror_stderr);
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    void write(LogLevel level, const std::string& message) const;
    static LogLevel parse_level(const std::string& name);

  private:
    LogLevel level_;
};
