#pragma once

#include <sstream>
#include <string>

namespace Util {

class Logger final {
public:
    class LogStream : public std::ostringstream {
        friend class Logger;

    public:
        LogStream(LogStream&) = delete;

        ~LogStream() {
            Logger::log(m_out, m_level.c_str(), m_logger_name.c_str(), str().c_str());
        }

    private:
        LogStream(FILE* output_file, std::string logger_name, std::string level)
            : m_logger_name(logger_name)
            , m_level(level)
            , m_out(output_file) {
        }

        const std::string m_logger_name;
        const std::string m_level;
        FILE* m_out;
    };

    explicit Logger(std::string name)
        : m_name(name) {
    }

    static void enable(bool enable);
    LogStream info() const { return LogStream(stdout, m_name, "INF"); }
    LogStream warning() const { return LogStream(stdout, m_name, "WAR"); }
    LogStream error() const { return LogStream(stderr, m_name, "ERR"); }

private:
    static void log(FILE*, const char* level, const char* name, const char* message);

    std::string m_name;
};

}

using Util::Logger;
