#include "json/json.h"

#include <cmath>
#include <sstream>

JSON::Result JSON::Element::Parse(ParsingContext& ctx, std::shared_ptr<Element>& out, bool allowDuplicateKeys, size_t maxDepth, size_t _depth)
{
    ctx.SkipWhitespaces();

    if (_depth >= maxDepth)
        return JSON_RESULT(ElementParseError, "Max depth of " + std::to_string(maxDepth) + " nested elements exceeded.", ctx);

    bool parsed = false; { // Try parse String
        auto str = std::make_shared<String>();
        auto res = String::Parse(ctx, *str);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(str);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Number
        auto num = std::make_shared<Number>();
        auto res = Number::Parse(ctx, *num);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(num);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Object
        auto obj = std::make_shared<Object>();
        auto res = Object::Parse(ctx, *obj, allowDuplicateKeys, maxDepth, _depth);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(obj);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Array
        auto arr = std::make_shared<Array>();
        auto res = Array::Parse(ctx, *arr, allowDuplicateKeys, maxDepth, _depth);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(arr);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Boolean
        auto boo = std::make_shared<Boolean>();
        auto res = Boolean::Parse(ctx, *boo);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(boo);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) { // Try parse Null
        auto nul = std::make_shared<Null>();
        auto res = Null::Parse(ctx, *nul);
        if (res == ResultStatus::OK) {
            parsed = true;
            out = std::move(nul);
        } else if (res != ResultStatus::ParsingAborted)
            return res;
    }

    if (!parsed) {
        // Common mistakes
        switch (ctx.Current()) {
        case '}':
            return JSON_RESULT(ElementParseError, "No Object to close.", ctx);
        case ']':
            return JSON_RESULT(ElementParseError, "No Array to close.", ctx);
        default:
            return JSON_RESULT(ElementParseError, "Expected String, Number, Boolean, Null, Object or Array.", ctx);
        }
    }

    ctx.SkipWhitespaces();

    return ResultStatus::OK;
}

std::string JSON::String::ToString() const
{
    std::string str;
    for (size_t i = 0; i < Value.length(); ++i) {
        char32_t ch = Value[i];
        if (IsControlCharacter(ch)) {
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
            str += EncodeUTF8(ch);
        }
    }
    return str;
}

std::string JSON::String::Serialize(bool pretty, size_t indent, char indentChar, size_t maxDepth, size_t _depth) const
{
    (void) pretty; (void) indent; (void) indentChar; (void) maxDepth; (void) _depth;
    return '"' + ToString() + '"';
}

JSON::Result JSON::String::Parse(ParsingContext& ctx, String& out)
{
    char32_t ch = ctx.Current();
    if (ch != '"')
        return ResultStatus::ParsingAborted;

    while (true) {
        if (!ctx.Next(ch)) {
            return JSON_RESULT(StringParseError, "String was not closed.", ctx);
        } else if (ch == '"') {
            break;
        } else if (IsControlCharacter(ch)) {
            return JSON_RESULT(StringParseError, "Expected character or end of string.", ctx);
        } else if (ch == '\\') {
            if (!ctx.Next(ch))
                return JSON_RESULT(StringParseError, "Escape sequence not provided.", ctx);
            switch (ch) {
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
                char16_t codepoint = 0;
                for (size_t i = 0; i < 4; i++) {
                    if (!ctx.Next(ch) || !IsHex(ch))
                        return JSON_RESULT(StringParseError, "Unicode escape sequence expected 4 hex digits.", ctx);
                    codepoint *= 16;
                    codepoint += IsDigit(ch) ? (ch - '0') : (ToLower(ch) - 'a' + 10);
                }
                
                out.Value += codepoint;
                // return JSON_RESULT(StringParseError, "Unicode characters outside the inclusive range [0, 255] are not supported.", ctx);
                continue;
            }
            default:
                return JSON_RESULT(StringParseError, "Invalid escape sequence provided.", ctx);
            }
        }
        out.Value += ch;
    }

    // Consume last "
    JSON_ASSERT(ch == '"');
    ctx.Next(ch);
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

JSON::Result JSON::Number::Parse(ParsingContext& ctx, Number& out)
{
    char32_t ch = ctx.Current();
    if (ch != '-' && !IsDigit(ch))
        return ResultStatus::ParsingAborted;
    
    bool isNegative = ch == '-';
    
    if (isNegative)
        ctx.Next(ch);

    int64_t integer = 0;
    if (ch != '0') {
        auto res = JSON::Number::ParseInteger(ctx, false, integer, nullptr);
        if (res != ResultStatus::OK)
            return res;
    } else ctx.Next(ch);

    ch = ctx.Current();
    bool isReal = ch == '.';
    double fraction = 0.0;
    if (isReal) {
        ctx.Next(ch);

        int64_t intFraction = 0;
        size_t digits = 0;

        auto res = JSON::Number::ParseInteger(ctx, false, intFraction, &digits);
        if (res != ResultStatus::OK)
            return res;
        
        fraction = (double)intFraction / std::pow<double>(10, digits);
        JSON_ASSERT(fraction < 1.0);
    }

    ch = ctx.Current();
    double exponent = 1.0;
    if (ch == 'e' || ch == 'E') {
        ctx.Next(ch);

        int64_t intExponent = 0;
        auto res = JSON::Number::ParseInteger(ctx, true, intExponent);
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

JSON::Result JSON::Number::ParseInteger(ParsingContext& ctx, bool allowSign, int64_t& out, size_t* digits)
{
    char32_t ch = ctx.Current();
    bool hasSign = ch == '-' || ch == '+';
    if (hasSign && !allowSign)
        return JSON_RESULT(NumberParseError, "Sign is not allowed here.", ctx);

    if (!hasSign && !IsDigit(ch))
        return ResultStatus::ParsingAborted;
    
    bool isNegative = ch == '-';
    if (isNegative || ch == '+')
        ctx.Next(ch);
    
    if (!IsDigit(ch))
        return JSON_RESULT(NumberParseError, "Expected digit, got character.", ctx);

    size_t digitCount = 0;
    int64_t integer = 0;

    for (; IsDigit(ch); ctx.Next(ch), ++digitCount) {
        int64_t digit = ch - '0';
        JSON_ASSERT(digit >= 0 && digit <= 9);
        integer *= 10;
        integer += digit;
    }

    out = isNegative ? -integer : integer;
    if (digits)
        *digits = digitCount;
    return ResultStatus::OK;
}

JSON::Result JSON::Boolean::Parse(ParsingContext& ctx, Boolean& out)
{
    if (ctx.Current() == 't') {
        std::u32string literal;
        if (!ctx.NextString(literal, 3) || literal != U"rue")
            return JSON_RESULT(BooleanParseError, "Expected true.", ctx);
        ctx.Next();
        out.Value = true;
        return ResultStatus::OK;
    } else if (ctx.Current() == 'f') {
        std::u32string literal;
        if (!ctx.NextString(literal, 4) || literal != U"alse")
            return JSON_RESULT(BooleanParseError, "Expected false.", ctx);
        ctx.Next();
        out.Value = false;
        return ResultStatus::OK;
    }
    return ResultStatus::ParsingAborted;
}

JSON::Result JSON::Null::Parse(ParsingContext& ctx, Null& out)
{
    (void) out;
    if (ctx.Current() == 'n') {
        std::u32string literal;
        if (!ctx.NextString(literal, 3) || literal != U"ull")
            return JSON_RESULT(NullParseError, "Expected null.", ctx);
        ctx.Next();
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

JSON::Result JSON::Object::Parse(ParsingContext& ctx, Object& out, bool allowDuplicateKeys, size_t maxDepth, size_t _depth)
{
    char32_t ch = ctx.Current();
    if (ch != '{')
        return ResultStatus::ParsingAborted;
    ctx.Next(ch);

    ctx.SkipWhitespaces();
    ch = ctx.Current();

    if (ch == '}') {
        ctx.Next(ch);
        return ResultStatus::OK;
    }

    while (true) {
        String key;
        { // Parse Key
            auto res = String::Parse(ctx, key);
            if (res != ResultStatus::OK)
                return res;
        }
        
        ctx.SkipWhitespaces();
        ch = ctx.Current();
        if (ch != ':')
            return JSON_RESULT(ObjectParseError, "Expected key-value pair, only key provided.", ctx);
        ctx.Next(ch);
        
        std::shared_ptr<Element> value = nullptr;
        { // Parse Value
            auto res = Element::Parse(ctx, value, allowDuplicateKeys, maxDepth, _depth+1);
            if (res != ResultStatus::OK)
                return res;
        }

        if (!allowDuplicateKeys && out.Value.find(key.Value) != out.Value.end())
            return JSON_RESULT(ObjectParseError, "Found duplicate key.", ctx);

        out.Value[key.Value] = value;

        ch = ctx.Current();
        if (ch == '}')
            break;
        else if (ch != ',')
            return JSON_RESULT(ObjectParseError, "Expected end of object '}' or ',' to define a new pair.", ctx);
        ctx.Next(ch);
        ctx.SkipWhitespaces();
    }

    JSON_ASSERT(ctx.Current() == '}');
    ctx.Next(ch);

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

JSON::Result JSON::Array::Parse(ParsingContext& ctx, Array& out, bool allowDuplicateKeys, size_t maxDepth, size_t _depth)
{
    char32_t ch = ctx.Current();
    if (ch != '[')
        return ResultStatus::ParsingAborted;
    ctx.Next(ch);

    ctx.SkipWhitespaces();
    ch = ctx.Current();
    if (ch == ']') {
        ctx.Next(ch);
        return ResultStatus::OK;
    }

    while (true) {
        std::shared_ptr<Element> value = nullptr;
        { // Parse Value
            auto res = Element::Parse(ctx, value, allowDuplicateKeys, maxDepth, _depth+1);
            if (res != ResultStatus::OK)
                return res;
        }

        out.Value.push_back(value);

        ch = ctx.Current();
        if (ch == ']')
            break;
        else if (ch != ',')
            return JSON_RESULT(ArrayParseError, "Expected end of array ']' or ',' to define a new value.", ctx);
        ctx.Next(ch);
    }

    JSON_ASSERT(ctx.Current() == ']');
    ctx.Next(ch);

    return ResultStatus::OK;
}

JSON::Result JSON::Parse(const std::string& json, std::shared_ptr<Element>& out, bool allowDuplicateKeys, size_t maxDepth)
{
    ParsingContext ctx(json);

    char32_t ch;
    if (!ctx.Next(ch))
        return JSON_RESULT(JSONParseError, "Empty JSON.", ctx);

    Result result = JSON::Element::Parse(ctx, out, allowDuplicateKeys, maxDepth);
    if (result && !ctx.IsEOF()) {
        ch = ctx.Current();
        // Common mistakes
        switch (ch) {
        case '}':
            return JSON_RESULT(JSONParseError, "No Object to close.", ctx);
        case ']':
            return JSON_RESULT(JSONParseError, "No Array to close.", ctx);
        default:
            return JSON_RESULT(JSONParseError, "Expected end of JSON.", ctx);
        }
    }
    return result;
}
