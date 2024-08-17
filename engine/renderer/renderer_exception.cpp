#include "renderer_exception.h"

#include <core/logger.h>

#include <cstdio>
#include <cstring>
#include <cstdarg>

RendererException::RendererException(const char* error, ...) {
    va_list va;
    va_start(va, error);                                                       
    m_What = new char[20000];
    memset(m_What, 0, 20000);                                              
    vsprintf(m_What, error, va);                                                  
    va_end(va);

    Logger::Fatal("%s", m_What);
}

RendererException::~RendererException() {
    delete[] m_What;
}