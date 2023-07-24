#pragma once

#include <cassert>

#define ENGINE_CRASH() assert(false)
#define ENGINE_CRASH_MESSAGE(message) assert(false && message)