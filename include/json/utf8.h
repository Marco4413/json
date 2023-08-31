#pragma once

#ifndef _JSON_UTF8_H
#define _JSON_UTF8_H

// Include miniutf8 under the JSON namespace
#include <cinttypes>
#include <cstring>
#include <string>
#include <string_view>

#define MINIUTF8_EXT_INCLUDE
#define MINIUTF8_NO_GUARDS

namespace JSON
{
    #include "miniutf8.hpp"
}

#endif // _JSON_UTF8_H
