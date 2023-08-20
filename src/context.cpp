#include "json/context.h"

bool JSON::ParsingContext::Next(char32_t& out)
{
    if (m_IsEndOfFile)
        return false;
    else if (!NextUTF8(out)) {
        out = '\0';
        m_Current = out;
        m_IsEndOfFile = true;
        return false;
    } else if (out == '\r' || out == '\n') {
        m_RowCursor = m_Cursor;
        ++m_Row;
        m_Column = 0;
        m_Current = out;
        return true;
    }

    ++m_Column;
    m_Current = out;
    return true;
}

bool JSON::ParsingContext::NextString(std::u32string& out, size_t count)
{
    out.resize(count);
    for (size_t i = 0; i < count; i++) {
        if (!Next(out[i]))
            return false;
    }
    return true;
}

void JSON::ParsingContext::SkipWhitespaces()
{
    char32_t ch = m_Current;
    while ((ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') && Next(ch));
}

std::string JSON::ParsingContext::GetLine() const
{
    if (m_Buffer.length() == 0)
        return "";
    size_t lineEnd = m_Buffer.find_first_of("\r\n", m_RowCursor);
    if (lineEnd == std::string::npos)
        return m_Buffer.substr(m_RowCursor);
    return m_Buffer.substr(m_RowCursor, lineEnd-m_RowCursor);
}

bool JSON::ParsingContext::NextChar(uint8_t& out)
{
    if (m_Cursor < m_Buffer.length()) {
        out = m_Buffer[m_Cursor++];
        return true;
    }
    return false;
}

// https://en.wikipedia.org/wiki/UTF-8
bool JSON::ParsingContext::NextUTF8(char32_t& out)
{
    bool isFirst = m_Cursor == 0;

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

    uint8_t ch;
    if (!NextChar(ch))
        return false;
    
    size_t byteCount = 0;
    if ((ch & BYTE1_MASK) == BYTE1) {
        out = ch;
        return true;
    } else if ((ch & BYTE2_MASK) == BYTE2) {
        out = ch & ~BYTE2_MASK;
        byteCount = 1;
    } else if ((ch & BYTE3_MASK) == BYTE3) {
        out = ch & ~BYTE3_MASK;
        byteCount = 2;
    } else if ((ch & BYTE4_MASK) == BYTE4) {
        out = ch & ~BYTE4_MASK;
        byteCount = 3;
    } else return false;

    for (size_t i = 0; i < byteCount; ++i) {
        if (!NextChar(ch) || (ch & BYTE_MASK) != BYTE)
            return false;
        out = (out << 6) | (ch & ~BYTE_MASK);
    }

    // If it's the BOM character read again.
    if (isFirst && out == 0xFEFF)
        return NextUTF8(out);
    return true;
}
