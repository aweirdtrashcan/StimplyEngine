#pragma once

#include <exception>

class EngineException : public std::exception
{
public:
    EngineException() = delete;
    EngineException(const char* reason, const char* type)
        : reason(reason), _type(type) {};

    virtual const char* what() const override { return reason; }
    virtual const char* type() const = 0;

protected:
    const char *reason, *_type;
};

