#pragma once

#ifndef _JSON_CONTEXT_H
#define _JSON_CONTEXT_H

#include <cinttypes>
#include <string>

namespace JSON
{
    // https://en.wikipedia.org/wiki/UTF-8
    inline static std::string EncodeUTF8(char32_t ch)
    {
        std::string encoded;
        if (ch > 0x10FFFF)
            ch = 0x10FFFF;

        if (ch <= 0x7F) {
            encoded += (char)ch;
        } else if (ch <= 0x07FF) {
            encoded += (char)(ch >> 6  ) | 0xC0;
            encoded += (char)(ch & 0x3F) | 0x80;
        } else if (ch <= 0xFFFF) {
            encoded += (char)( ch >> 12        ) | 0xE0;
            encoded += (char)((ch >> 6 ) & 0x3F) | 0x80;
            encoded += (char)( ch        & 0x3F) | 0x80;
        } else if (ch <= 0x10FFFF) {
            encoded += (char)( ch >> 18        ) | 0xF0;
            encoded += (char)((ch >> 12) & 0x3F) | 0x80;
            encoded += (char)((ch >> 6 ) & 0x3F) | 0x80;
            encoded += (char)( ch        & 0x3F) | 0x80;
        }

        return encoded;
    }

    inline static std::string EncodeUTF8(const std::u32string& str)
    {
        std::string encoded;
        for (size_t i = 0; i < str.length(); i++)
            encoded += EncodeUTF8(str[i]);
        return encoded;
    }

    class ParsingContext
    {
    public:
        ParsingContext(const std::string& in)
            : m_Buffer(in) { }
        
        bool Next() { char32_t ch; return Next(ch); }
        bool Next(char32_t& out);
        bool NextString(std::u32string& out, size_t count);
        void SkipWhitespaces();

        char32_t Current() const { return m_Current; }
        bool IsEOF() const { return m_IsEndOfFile; }
        
        uint32_t GetRow() const { return m_Row; }
        uint32_t GetColumn() const { return m_Column; }
        std::string GetLine() const;

    private:
        bool NextChar(uint8_t& out);
        bool NextUTF8(char32_t& out);

        size_t m_Cursor = 0;
        size_t m_RowCursor = 0;
        const std::string& m_Buffer;
        bool m_IsEndOfFile = false;

        uint32_t m_Row = 0;
        uint32_t m_Column = 0;
        char32_t m_Current = 0;
    };
}

#endif // _JSON_CONTEXT_H
