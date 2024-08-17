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
    static void Fatal(const char* format, ...);
    static void Warning(const char* format, ...);
    static void Debug(const char* format, ...);
    static void Info(const char* format, ...);
};