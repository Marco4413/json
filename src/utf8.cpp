#include "json/utf8.h"

// https://en.wikipedia.org/wiki/UTF-8
std::string JSON::EncodeUTF8(char32_t ch)
{
    std::string encoded;
    encoded.reserve(4);
    if (ch > UTF8::NOT_A_CHARACTER)
        ch = UTF8::NOT_A_CHARACTER;

    if (ch <= 0x7F) {
        encoded += (char)ch;
    } else if (ch <= 0x07FF) {
        encoded += (char)(ch >> 6) | UTF8::BYTE2;
        encoded += (char)(ch & ~UTF8::BYTE_MASK) | UTF8::BYTE;
    } else if (ch <= 0xFFFF) {
        encoded += (char)(ch >> 12) | UTF8::BYTE3;
        encoded += (char)((ch >> 6) & ~UTF8::BYTE_MASK) | UTF8::BYTE;
        encoded += (char)( ch       & ~UTF8::BYTE_MASK) | UTF8::BYTE;
    } else if (ch <= 0x10FFFF) {
        encoded += (char)(ch >> 18) | UTF8::BYTE4;
        encoded += (char)((ch >> 12) & ~UTF8::BYTE_MASK) | UTF8::BYTE;
        encoded += (char)((ch >> 6 ) & ~UTF8::BYTE_MASK) | UTF8::BYTE;
        encoded += (char)( ch        & ~UTF8::BYTE_MASK) | UTF8::BYTE;
    }

    return encoded;
}

std::string JSON::EncodeUTF8(const std::u32string& str)
{
    std::string encoded;
    encoded.reserve(str.length());
    for (size_t i = 0; i < str.length(); i++)
        encoded += EncodeUTF8(str[i]);
    return encoded;
}

char32_t JSON::UTF8StringDecoder::Next()
{
    if (!(*this))
        return UTF8::NOT_A_CHARACTER;

    char32_t codepoint = 0;
    size_t byteCount = 0;

    uint8_t ch = *(m_Cursor++);
    if ((ch & UTF8::BYTE1_MASK) == UTF8::BYTE1) {
        return ch;
    } else if ((ch & UTF8::BYTE2_MASK) == UTF8::BYTE2) {
        codepoint = (ch & ~UTF8::BYTE2_MASK);
        byteCount = 1;
    } else if ((ch & UTF8::BYTE3_MASK) == UTF8::BYTE3) {
        codepoint = (ch & ~UTF8::BYTE3_MASK);
        byteCount = 2;
    } else if ((ch & UTF8::BYTE4_MASK) == UTF8::BYTE4) {
        codepoint = (ch & ~UTF8::BYTE4_MASK);
        byteCount = 3;
    } else
        return UTF8::NOT_A_CHARACTER;

    for (size_t i = 0; i < byteCount; ++i) {
        if ((m_Cursor + i) >= m_End) {
            m_Cursor = m_End;
            return UTF8::NOT_A_CHARACTER;
        } else if ((m_Cursor[i] & UTF8::BYTE_MASK) != UTF8::BYTE) {
            m_Cursor += i+1;
            return UTF8::NOT_A_CHARACTER;
        }
        codepoint = (codepoint << 6) | (uint8_t)(m_Cursor[i] & ~UTF8::BYTE_MASK);
    }

    m_Cursor += byteCount;
    return codepoint;
}

std::u32string JSON::DecodeUTF8(const std::string& str)
{
    std::u32string decoded;
    UTF8StringDecoder decoder(str);
    while (decoder)
        decoded += decoder.Next();
    return decoded;
}
