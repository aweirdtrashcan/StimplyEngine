#pragma once

enum log_level : char {
    fatal,
    warning,
    debug,
    info
};

class RAPI Logger {
public:
    static void InitializeLogging();
    static void ShutdownLogging();
    static void fatal(const char* format, ...);
    static void warning(const char* format, ...);
    static void debug(const char* format, ...);
    static void info(const char* format, ...);
};