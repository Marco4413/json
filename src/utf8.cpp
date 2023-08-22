#include "json/utf8.h"

#include <cstring>

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

size_t JSON::UTF8::Length(const std::string& str)
{
    size_t len = 0;
    UTF8StringDecoder decoder(str);
    for (; decoder; decoder.Next(), ++len);
    return len;
}

std::string& JSON::UTF8::Erase(std::string& target, size_t pos, size_t len)
{
    UTF8StringDecoder decoder(target);

    for (size_t i = 0; i < pos && decoder; decoder.Next(), ++i);
    const char* eraseStart = decoder.GetCursor();

    for (size_t i = 0; i < len && decoder; decoder.Next(), ++i);
    const char* eraseEnd = decoder.GetCursor();

    size_t erasepos = eraseStart - target.c_str();
    size_t eraselen = eraseEnd - eraseStart;
    return target.erase(erasepos, eraselen);
}

std::string& JSON::UTF8::Insert(std::string& target, size_t pos, const std::string& str)
{
    return Insert(target, pos, str.c_str(), str.length());
}

std::string& JSON::UTF8::Insert(std::string& target, size_t pos, const std::string& str, size_t subpos, size_t sublen)
{
    UTF8StringDecoder decoder(str);

    const char* substart;
    for (size_t i = 0; i < subpos && decoder; decoder.Next(), ++i);
    substart = decoder.GetCursor();

    const char* subend;
    for (size_t i = 0; i < sublen && decoder; decoder.Next(), ++i);
    subend = decoder.GetCursor();

    return Insert(target, pos, substart, subend-substart);
}

std::string& JSON::UTF8::Insert(std::string& target, size_t pos, const char* s)
{
    return Insert(target, pos, s, std::strlen(s));
}

std::string& JSON::UTF8::Insert(std::string& target, size_t pos, const char* s, size_t n)
{
    UTF8StringDecoder decoder(target);
    for (size_t i = 0; i < pos && decoder; decoder.Next(), ++i);

    const char* insertCursor = decoder.GetCursor();
    size_t insertAt = insertCursor-target.c_str();
    return target.insert(insertAt, s, n);
}

std::string& JSON::UTF8::Insert(std::string& target, size_t pos, size_t n, char c)
{
    return Insert(target, pos, std::string(n, c));
}

std::string& JSON::UTF8::Replace(std::string& target, size_t pos, size_t len, const std::string& str)
{
    return Replace(target, pos, len, str, 0);
}

std::string& JSON::UTF8::Replace(std::string& target, size_t pos, size_t len, const std::string& str, size_t subpos, size_t sublen)
{
    return Insert(Erase(target, pos, len), pos, str, subpos, sublen);
}

std::string& JSON::UTF8::Replace(std::string& target, size_t pos, size_t len, const char* s)
{
    return Replace(target, pos, len, s, std::strlen(s));
}

std::string& JSON::UTF8::Replace(std::string& target, size_t pos, size_t len, const char* s, size_t n)
{
    return Insert(Erase(target, pos, len), pos, s, n);
}

std::string& JSON::UTF8::Replace(std::string& target, size_t pos, size_t len, size_t n, char c)
{
    return Replace(target, pos, len, std::string(n, c));
}
