#include "logger.hpp"
#include <stdexcept>
#include <syslog.h>

namespace {
int priority(LogLevel level) {
    switch (level) {
    case LogLevel::error: return LOG_ERR;
    case LogLevel::warning: return LOG_WARNING;
    case LogLevel::info: return LOG_INFO;
    case LogLevel::debug: return LOG_DEBUG;
    }
    return LOG_ERR;
}
}

Logger::Logger(LogLevel level, bool mirror_stderr) : level_(level) {
    openlog("device-gateway", LOG_PID | LOG_NDELAY | (mirror_stderr ? LOG_PERROR : 0), LOG_DAEMON);
    setlogmask(LOG_UPTO(priority(level)));
}

Logger::~Logger() { closelog(); }

void Logger::write(LogLevel level, const std::string& message) const {
    if (priority(level) > priority(level_)) return;
    // Keep untrusted UART/TCP payloads on one bounded log line.
    std::string clean = message.substr(0, 1024);
    for (char& ch : clean) {
        if (static_cast<unsigned char>(ch) < 32 || ch == 127) ch = ' ';
    }
    syslog(priority(level), "%s", clean.c_str());
}

LogLevel Logger::parse_level(const std::string& name) {
    if (name == "error") return LogLevel::error;
    if (name == "warning") return LogLevel::warning;
    if (name == "info") return LogLevel::info;
    if (name == "debug") return LogLevel::debug;
    throw std::runtime_error("Invalid log level: " + name);
}
