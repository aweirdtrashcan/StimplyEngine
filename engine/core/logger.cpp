#include "logger.h"

#include "platform/platform.h"

#include <cstdarg>
#include <stdio.h>

#define internal_logger(level, format)                                                                      \
    va_list va;                                                                                             \
    va_start(va, format);                                                                                   \
    char out_arg_message[10000];                                                                            \
    vsprintf(out_arg_message, format, va);                                                                  \
    va_end(va);                                                                                             \
\
    static constexpr const char* level_strings[] = { "[FATAL]: ", "[WARN]: ", "[DEBUG]: ", "[INFO]: " };   \
\
    char output_message[11000];                                                                             \
    snprintf(output_message, sizeof(output_message) - 1, "%s%s", level_strings[level], out_arg_message);  \
\
    Platform::log(level, output_message);                                                                   \

void Logger::fatal(const char* format, ...) {
    internal_logger(log_level::fatal, format);
}

void Logger::warning(const char* format, ...) {
    internal_logger(log_level::warning, format);
}

void Logger::debug(const char* format, ...) {
    internal_logger(log_level::debug, format);
}

void Logger::info(const char* format, ...) {
    internal_logger(log_level::info, format);
}