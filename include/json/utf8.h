#pragma once

#ifndef _JSON_UTF8_H
#define _JSON_UTF8_H

#include <cinttypes>
#include <string>

namespace JSON
{
    namespace UTF8
    {
        constexpr uint8_t BYTE1 = 0x00;
        constexpr uint8_t BYTE1_MASK = 0x80;
        constexpr uint8_t BYTE2 = 0xC0;
        constexpr uint8_t BYTE2_MASK = 0xE0;
        constexpr uint8_t BYTE3 = 0xE0;
        constexpr uint8_t BYTE3_MASK = 0xF0;
        constexpr uint8_t BYTE4 = 0xF0;
        constexpr uint8_t BYTE4_MASK = 0xF8;

        constexpr uint8_t BYTE = 0x80;
        constexpr uint8_t BYTE_MASK = 0xC0;

        constexpr char32_t BOM = 0xFEFF;
        constexpr char32_t NOT_A_CHARACTER = 0x10FFFF;
    }

    std::string EncodeUTF8(char32_t ch);
    std::string EncodeUTF8(const std::u32string& str);

    class UTF8StringDecoder
    {
    public:
        UTF8StringDecoder(const char* begin, const char* end)
            : m_Cursor(begin > end ? end : begin), m_End(end) { }

        UTF8StringDecoder(const std::string& str, size_t offset)
            : UTF8StringDecoder(str.c_str()+offset, str.c_str()+str.length()) { }

        UTF8StringDecoder(const std::string& str)
            : UTF8StringDecoder(str, 0) { }
        
        const char* GetCursor() const { return m_Cursor; }
        const char* GetEnd() const { return m_End; }

        char32_t Next();
        operator bool() const { return m_Cursor < m_End; }

    private:
        const char* m_Cursor;
        const char* const m_End;
    };

    std::u32string DecodeUTF8(const std::string& str);
}

#endif // _JSON_UTF8_H
