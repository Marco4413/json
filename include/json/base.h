#pragma once

#ifndef _JSON_BASE_H
#define _JSON_BASE_H

#include <cstdio>
#include <cstdlib>

#include "json/context.h"

#define JSON_UNREACHABLE() \
    std::exit(1)

#ifdef JSON_DEBUG

#define JSON_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            std::printf("%s:%d: Assert (%s) failed.\n", __FILE__, __LINE__, #cond); \
            std::exit(1); \
        } \
    } while (false)

#define JSON_RESULT(status, error, ctx) \
    JSON::Result(JSON::ResultStatus:: status, std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " + (error), (ctx))

#else // JSON_DEBUG

#define JSON_ASSERT(cond) \
    (void)(cond)

#define JSON_RESULT(status, error, ctx) \
    JSON::Result(JSON::ResultStatus:: status, (error), (ctx))

#endif // JSON_DEBUG

namespace JSON
{
    // https://www.ecma-international.org/publications-and-standards/standards/ecma-404/
    inline static bool IsControlCharacter(char32_t ch) { return /* ch >= 0x00 && */ ch <= 0x1F; }
    inline static char32_t ToLower(char32_t ch) { return ch >= 0x41 && ch <= 0x5A ? (ch + 32) : ch; }
    inline static bool IsDigit(char32_t ch) { return ch >= 0x30 && ch <= 0x39; }
    inline static bool IsHex(char32_t ch) { return IsDigit(ch) || (ToLower(ch) >= 0x61 && ToLower(ch) <= 0x66); }

    enum class ResultStatus
    {
        OK,
        ParsingAborted,
        JSONParseError,
        ElementParseError,
        ObjectParseError,
        ArrayParseError,
        StringParseError,
        NumberParseError,
        BooleanParseError,
        NullParseError,
    };

    struct Result
    {
    public:
        const ResultStatus Status;
        const std::string Error;

        const uint32_t ErrorRow = 0;
        const uint32_t ErrorColumn = 0;
        const std::string ErrorLine = "";
    
    public:
        Result(ResultStatus status, std::string_view error, const ParsingContext& ctx)
            : Status(status), Error(error), ErrorRow(ctx.GetRow()), ErrorColumn(ctx.GetColumn()), ErrorLine(ctx.GetLine()) { }

        Result(ResultStatus status, std::string_view error)
            : Status(status), Error(error) { }
        
        Result(ResultStatus status)
            : Result(status, "") { }

        std::string GetPrettyError(uint32_t viewRadius = 10) const;
        
        bool operator==(ResultStatus status) const { return Status == status; }
        bool operator!=(ResultStatus status) const { return Status != status; }

        operator bool() const { return Status == ResultStatus::OK; }
    };
}

#endif // _JSON_BASE_H
