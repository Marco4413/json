#include "json/json.h"

#include <cmath>
#include <cstring>
#include <sstream>

std::string JSON::Result::GetPrettyError(const char* json, size_t viewRadius) const
{
    if (*this)
        return "";
    else if (Token == nullptr)
        return Error;

    const char* token = Token;
    size_t line = 1;
    size_t lineChar = 0;
    for (const char* it = json; it < token; ++it) {
        if (*it == '\r' || *it == '\n') {
            lineChar = 0;
            ++line;
            continue;
        }
        ++lineChar;
    }

    const char* lineStart = token-lineChar;
    for (size_t i = 0; i <= viewRadius && !(*token == '\r' || *token == '\n' || *token == '\0'); ++i, ++token);

    auto lineNumber = std::to_string(line);
    auto lineCharNumber = std::to_string(lineChar+1);

    std::string errorLine(lineStart, token);
    for (size_t i = 0; i < errorLine.length(); i++) {
        // Remove any control character (CR-LF should not be present)
        JSON_ASSERT(errorLine[i] != '\r' && errorLine[i] != '\n');
        if (iscntrl((unsigned char)errorLine[i]))
            errorLine[i] = ' ';
    }

    size_t trimmedFromError = TrimAllPrevious(errorLine, lineChar, viewRadius);

    std::string error;
    error += std::string(lineNumber.length(), ' ') + " | " + Error + '\n';

    error += lineNumber + " | ";
    if (trimmedFromError > 0)
        error += "... ";
    error += errorLine;
    if (*token != '\0')
        error += " ...";
    error += '\n';

    std::string finder(lineChar-trimmedFromError, ' ');
    finder += '^';
    finder += '~' + lineCharNumber;
    error += std::string(lineNumber.length(), ' ') + " | ";
    if (trimmedFromError > 0)
        error += "    ";
    error += finder;

    return error;
}

JSON::Result JSON::Element::Parse(const char** buf, std::shared_ptr<Element>& out, bool allowDuplicateKeys, size_t maxDepth, size_t _depth)
{
    SkipWhitespaces(buf);

    if (_depth >= maxDepth)
        return JSON_RESULT(ElementParseError, "Max depth of " + std::to_string(maxDepth) + " nested elements exceeded.", *buf);

    bool parsed = false; { // Try parse String
        auto str = std::make_shared<String>();
        auto res = String::Parse(buf, *str);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(str);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Number
        auto num = std::make_shared<Number>();
        auto res = Number::Parse(buf, *num);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(num);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Object
        auto obj = std::make_shared<Object>();
        auto res = Object::Parse(buf, *obj, allowDuplicateKeys, maxDepth, _depth);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(obj);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Array
        auto arr = std::make_shared<Array>();
        auto res = Array::Parse(buf, *arr, allowDuplicateKeys, maxDepth, _depth);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(arr);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Boolean
        auto boo = std::make_shared<Boolean>();
        auto res = Boolean::Parse(buf, *boo);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(boo);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Null
        auto nul = std::make_shared<Null>();
        auto res = Null::Parse(buf, *nul);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(nul);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) {
        // Common mistakes
        switch (**buf) {
        case '}':
            return JSON_RESULT(ElementParseError, "No Object to close.", *buf);
        case ']':
            return JSON_RESULT(ElementParseError, "No Array to close.", *buf);
        default:
            return JSON_RESULT(ElementParseError, "Expected String, Number, Boolean, Null, Object or Array.", *buf);
        }
    }

    SkipWhitespaces(buf);

    return ResultStatus::OK;
}

std::string JSON::String::Serialize(bool pretty, size_t indent, char indentChar, size_t maxDepth, size_t _depth) const
{
    (void) pretty; (void) indent; (void) indentChar; (void) maxDepth; (void) _depth;
    std::string str;
    for (size_t i = 0; i < Value.length(); ++i) {
        unsigned char ch = Value[i];
        if (iscntrl(ch)) {
            str += '\\';
            switch (ch) {
            case '\b':
                str += 'b';
                break;
            case '\f':
                str += 'f';
                break;
            case '\n':
                str += 'n';
                break;
            case '\r':
                str += 'r';
                break;
            case '\t':
                str += 't';
                break;
            default: {
                str += 'u';
                char hex[] = "0000";
                for (size_t i = 1; i <= 4; ++i) {
                    uint8_t mod = ch % 16;
                    ch /= 16;
                    char digit = mod < 10 ? '0' + mod : 'a' + (mod - 10);
                    hex[4-i] = digit;
                }
                str += hex;
            }
            }
            continue;
        }

        switch (ch) {
        case '"':
        case '\\':
        case '/':
            str += '\\';
            [[fallthrough]];
        default:
            str += ch;
        }
    }
    return '"' + str + '"';
}

JSON::Result JSON::String::Parse(const char** _buf, String& out)
{
    const char*& buf = *_buf;

    if (*buf != '"')
        return ResultStatus::ParsingAborted;

    while (*(++buf) != '"') {
        if (*buf == '\0') {
            return JSON_RESULT(StringParseError, "String was not closed.", buf);
        } else if (IsControlCharacter((unsigned char)*buf)) {
            return JSON_RESULT(StringParseError, "Expected character or end of string.", buf);
        } else if (*buf == '\\') {
            if (*(++buf) == '\0')
                return JSON_RESULT(StringParseError, "Escape sequence not provided.", buf);
            switch (*buf) {
            case '"':
            case '\\':
            case '/':
                break;
            case 'b':
                out.Value += '\b';
                continue;
            case 'f':
                out.Value += '\f';
                continue;
            case 'n':
                out.Value += '\n';
                continue;
            case 'r':
                out.Value += '\r';
                continue;
            case 't':
                out.Value += '\t';
                continue;
            case 'u': {
                uint16_t codepoint = 0;
                for (size_t i = 0; i < 4; i++) {
                    unsigned char hex = tolower(*(++buf));
                    if ((hex < 'a' || hex > 'f') && !isdigit(hex))
                        return JSON_RESULT(StringParseError, "Unicode escape sequence expected 4 hex digits.", buf);
                    codepoint *= 16;
                    codepoint += isdigit(hex) ? (hex - '0') : (hex - 'a' + 10);
                }
                
                if (codepoint <= 255) {
                    out.Value += (char)codepoint;
                    continue;
                }
                
                return JSON_RESULT(StringParseError, "Unicode characters outside the inclusive range [0, 255] are not supported.", buf);
            }
            default:
                return JSON_RESULT(StringParseError, "Invalid escape sequence provided.", buf);
            }
        }
        out.Value += *buf;
    }

    ++buf; // Consume last "
    return ResultStatus::OK;
}

std::string JSON::Number::Serialize(bool pretty, size_t indent, char indentChar, size_t maxDepth, size_t _depth) const
{
    (void) pretty; (void) indent; (void) indentChar; (void) maxDepth; (void) _depth;
    if (m_IsReal) {
        // Removes unecessary decimal places and uses exponential notation when needed
        std::ostringstream ss;
        ss << m_Real;
        return ss.str();
    }

    return std::to_string(m_Integer);
}

JSON::Result JSON::Number::Parse(const char** _buf, Number& out)
{
    const char*& buf = *_buf;

    if (*buf != '-' && !isdigit((unsigned char)*buf))
        return ResultStatus::ParsingAborted;
    
    bool isNegative = *buf == '-';
    if (isNegative)
        ++buf;

    int64_t integer = 0;
    if (*buf != '0') {
        auto res = JSON::Number::ParseInteger(_buf, false, integer, nullptr);
        if (res != ResultStatus::OK)
            return res;
    } else ++buf;

    bool isReal = *buf == '.';
    double fraction = 0.0;
    if (isReal) {
        ++buf;

        int64_t intFraction = 0;
        size_t digits = 0;

        auto res = JSON::Number::ParseInteger(_buf, false, intFraction, &digits);
        if (res != ResultStatus::OK)
            return res;
        
        fraction = (double)intFraction / std::pow<double>(10, digits);
        JSON_ASSERT(fraction < 1.0);
    }

    double exponent = 1.0;
    if (*buf == 'e' || *buf == 'E') {
        ++buf;

        int64_t intExponent = 0;
        auto res = JSON::Number::ParseInteger(_buf, true, intExponent);
        if (res != ResultStatus::OK)
            return res;
        
        isReal = isReal || intExponent < 0;
        exponent = std::pow<double>(10, intExponent);
    }

    if (isReal) {
        out.SetReal(((double)integer + fraction) * exponent);
        return ResultStatus::OK;
    }

    out.SetInteger(integer * exponent);
    return ResultStatus::OK;
}

JSON::Result JSON::Number::ParseInteger(const char** _buf, bool allowSign, int64_t& out, size_t* digits)
{
    const char*& buf = *_buf;

    bool hasSign = *buf == '-' || *buf == '+';
    if (hasSign && !allowSign)
        return JSON_RESULT(NumberParseError, "Sign is not allowed here.", buf);

    if (!hasSign && !isdigit((unsigned char)*buf))
        return ResultStatus::ParsingAborted;
    
    bool isNegative = *buf == '-';
    if (isNegative || *buf == '+')
        ++buf;
    
    if (!isdigit((unsigned char)*buf))
        return JSON_RESULT(NumberParseError, "Expected digit, got character.", buf);

    size_t digitCount = 0;
    int64_t integer = 0;

    for (; isdigit((unsigned char)*buf); ++buf, ++digitCount) {
        int64_t digit = *buf - '0';
        JSON_ASSERT(digit >= 0 && digit <= 9);
        integer *= 10;
        integer += digit;
    }

    out = isNegative ? -integer : integer;
    if (digits)
        *digits = digitCount;
    return ResultStatus::OK;
}

JSON::Result JSON::Boolean::Parse(const char** buf, Boolean& out)
{
    if (strncmp(*buf, "true", 4) == 0) {
        *buf += 4;
        out.Value = true;
        return ResultStatus::OK;
    } else if (strncmp(*buf, "false", 5) == 0) {
        *buf += 5;
        out.Value = false;
        return ResultStatus::OK;
    }
    return ResultStatus::ParsingAborted;
}

JSON::Result JSON::Null::Parse(const char** buf, Null& out)
{
    (void) out;
    if (strncmp(*buf, "null", 4) == 0) {
        *buf += 4;
        return ResultStatus::OK;
    }
    return ResultStatus::ParsingAborted;
}

std::string JSON::Object::Serialize(bool pretty, size_t indent, char indentChar, size_t maxDepth, size_t _depth) const
{
    if (_depth >= maxDepth || Value.size() == 0)
        return "{}";

    std::string lineStart = "\n";
    if (pretty)
        lineStart += std::string((_depth+1) * indent, indentChar);

    std::string str;
    for (const auto& kv : Value) {
        if (pretty)
            str += lineStart;
        String key(kv.first);
        str += key.Serialize(pretty, indent, indentChar, maxDepth, _depth);
        str += ": ";
        str += kv.second->Serialize(pretty, indent, indentChar, maxDepth, _depth+1);
        str += ',';
    }

    // Remove comma
    if (!str.empty())
        str.pop_back();
    if (pretty) {
        str += '\n';
        if (_depth > 0)
            str.append(lineStart.c_str(), 1, lineStart.length()-indent-1);
    }
    return '{' + str + '}';
}

JSON::Result JSON::Object::Parse(const char** _buf, Object& out, bool allowDuplicateKeys, size_t maxDepth, size_t _depth)
{
    const char*& buf = *_buf;

    if (*buf != '{')
        return ResultStatus::ParsingAborted;
    ++buf;

    SkipWhitespaces(_buf);
    if (*buf == '}') {
        ++buf;
        return ResultStatus::OK;
    }

    while (true) {
        String key;
        { // Parse Key
            auto res = String::Parse(_buf, key);
            if (res != ResultStatus::OK)
                return res;
        }
        
        SkipWhitespaces(_buf);
        if (*buf != ':')
            return JSON_RESULT(ObjectParseError, "Expected key-value pair, only key provided.", buf);
        ++buf;
        
        std::shared_ptr<Element> value = nullptr;
        { // Parse Value
            auto res = Element::Parse(_buf, value, allowDuplicateKeys, maxDepth, _depth+1);
            if (res != ResultStatus::OK)
                return res;
        }

        if (!allowDuplicateKeys && out.Value.find(key.Value) != out.Value.end())
            return JSON_RESULT(ObjectParseError, "Found duplicate key.", buf);

        out.Value[key.Value] = value;

        if (*buf == '}')
            break;
        else if (*buf != ',')
            return JSON_RESULT(ObjectParseError, "Expected end of object '}' or ',' to define a new pair.", buf);
        ++buf;
        SkipWhitespaces(_buf);
    }

    JSON_ASSERT(*buf == '}');

    ++buf;
    return ResultStatus::OK;
}

std::string JSON::Array::Serialize(bool pretty, size_t indent, char indentChar, size_t maxDepth, size_t _depth) const
{
    if (_depth >= maxDepth || Value.size() == 0)
        return "[]";

    std::string lineStart = "\n";
    if (pretty)
        lineStart += std::string((_depth+1) * indent, indentChar);

    std::string str;
    for (const auto& el : Value) {
        if (pretty)
            str += lineStart;
        str += el->Serialize(pretty, indent, indentChar, maxDepth, _depth+1);
        str += ",";
    }

    // Remove comma
    if (!str.empty())
        str.pop_back();
    if (pretty) {
        str += '\n';
        if (_depth > 0)
            str.append(lineStart.c_str(), 1, lineStart.length()-indent-1);
    }
    return '[' + str + ']';
}

JSON::Result JSON::Array::Parse(const char** _buf, Array& out, bool allowDuplicateKeys, size_t maxDepth, size_t _depth)
{
    const char*& buf = *_buf;

    if (*buf != '[')
        return ResultStatus::ParsingAborted;
    ++buf;

    SkipWhitespaces(_buf);
    if (*buf == ']') {
        ++buf;
        return ResultStatus::OK;
    }

    while (true) {
        std::shared_ptr<Element> value = nullptr;
        { // Parse Value
            auto res = Element::Parse(_buf, value, allowDuplicateKeys, maxDepth, _depth+1);
            if (res != ResultStatus::OK)
                return res;
        }

        out.Value.push_back(value);

        if (*buf == ']')
            break;
        else if (*buf != ',')
            return JSON_RESULT(ArrayParseError, "Expected end of array ']' or ',' to define a new value.", buf);
        ++buf;
    }

    JSON_ASSERT(*buf == ']');

    ++buf;
    return ResultStatus::OK;
}

JSON::Result JSON::Parse(const char* buf, std::shared_ptr<Element>& out, bool allowDuplicateKeys, size_t maxDepth)
{
    Result result = JSON::Element::Parse(&buf, out, allowDuplicateKeys, maxDepth);
    if (result && *buf != '\0') {
        // Common mistakes
        switch (*buf) {
        case '}':
            return JSON_RESULT(JSONParseError, "No Object to close.", buf);
        case ']':
            return JSON_RESULT(JSONParseError, "No Array to close.", buf);
        default:
            return JSON_RESULT(JSONParseError, "Expected end of JSON.", buf);
        }
    }
    return result;
}
