#pragma once

#ifndef _JSON_CONTEXT_H
#define _JSON_CONTEXT_H

#include "json/utf8.h"

namespace JSON
{
    class ParsingContext
    {
    public:
        ParsingContext(std::string_view in)
            : m_Decoder(in), m_RowCursor(m_Decoder.GetCursor()) { }
        
        bool Next() { char32_t ch; return Next(ch); }
        bool Next(char32_t& out);
        bool NextString(std::u32string& out, size_t count);
        void SkipWhitespaces();

        char32_t Current() const { return m_Current; }
        bool IsEOF() const
        {
            // Cannot use !m_Decoder to check for end of file because it is always 1 character ahead.
            // i.e. If parsing "[]," , it would be valid because m_Decoder is looking at the character after ',' (which is EOF).
            return m_IsEndOfFile;
        }
        
        uint32_t GetRow() const { return m_Row; }
        uint32_t GetColumn() const { return m_Column; }
        std::string GetLine() const;

    private:
        bool NextUTF8(char32_t& out);

        UTF8::StringDecoder m_Decoder;
        const char* m_RowCursor;
        bool m_IsFirst = true;
        bool m_IsEndOfFile = false;

        uint32_t m_Row = 0;
        uint32_t m_Column = 0;
        char32_t m_Current = 0;
    };
}

#endif // _JSON_CONTEXT_H
