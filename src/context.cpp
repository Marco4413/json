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
        m_RowCursor = m_Decoder.GetCursor();
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
    const char* lineEnd = m_RowCursor;
    for (; lineEnd < m_Decoder.GetEnd(); ++lineEnd) {
        if (*lineEnd == '\r' || *lineEnd == '\n')
            break;
    }
    return std::string(m_RowCursor, lineEnd);
}

bool JSON::ParsingContext::NextUTF8(char32_t& out)
{
    if (!m_Decoder)
        return false;

    out = m_Decoder.Next();

    // If it's the BOM character read again.
    if (m_IsFirst && out == UTF8::BOM) {
        m_IsFirst = false;
        return NextUTF8(out);
    }

    return true;
}
