#include <core/test.h>
#include <core/logger.h>

int main(void) {
    Logger::fatal("This is a test message!");
    Logger::warning("This is a test message!");
    Logger::debug("This is a test message!");
    Logger::info("This is a test message!");

    initialize_engine();

    return 0;
}