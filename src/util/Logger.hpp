/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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
