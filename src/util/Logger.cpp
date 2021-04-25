#include "Logger.hpp"
#include <stdio.h>
#include <termios.h>

using namespace Util;

static bool s_enable = false;

void Logger::enable(bool enable) {
    s_enable = enable;
}

void Logger::log(FILE* output_file, const char* level, const char* name, const char* message) {
    if (s_enable) {
        fprintf(output_file, "[%s] (%s): %s\n", level, name, message);
        fflush(output_file);
    }
}
