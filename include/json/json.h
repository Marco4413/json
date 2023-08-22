#pragma once

#ifndef _JSON_H
#define _JSON_H

#include "json/base.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace JSON
{
    enum class ElementType
    {
        Object, Array, String, Number, Boolean, Null,
    };
    
    constexpr const char* ElementTypeToString(ElementType type)
    {
        switch (type) {
        case ElementType::Object:
            return "Object";
        case ElementType::Array:
            return "Array";
        case ElementType::String:
            return "String";
        case ElementType::Number:
            return "Number";
        case ElementType::Boolean:
            return "Boolean";
        case ElementType::Null:
            return "Null";
        default:
            JSON_UNREACHABLE();
        }
    }

    // Forward declaration for JSON Elements
    class Object;
    class Array;
    class String;
    class Number;
    class Boolean;
    class Null;

    class Element
    {
    public:
        Element() = delete;
        virtual ~Element() = default;

        ElementType GetType() const { return m_Type; }

        Object& AsObject() { JSON_ASSERT(m_Type == ElementType::Object); return *((Object*)this); }
        const Object& AsObject() const { JSON_ASSERT(m_Type == ElementType::Object); return *((Object*)this); }

        Array& AsArray() { JSON_ASSERT(m_Type == ElementType::Array); return *((Array*)this); }
        const Array& AsArray() const { JSON_ASSERT(m_Type == ElementType::Array); return *((Array*)this); }

        String& AsString() { JSON_ASSERT(m_Type == ElementType::String); return *((String*)this); }
        const String& AsString() const { JSON_ASSERT(m_Type == ElementType::String); return *((String*)this); }

        Number& AsNumber() { JSON_ASSERT(m_Type == ElementType::Number); return *((Number*)this); }
        const Number& AsNumber() const { JSON_ASSERT(m_Type == ElementType::Number); return *((Number*)this); }

        Boolean& AsBoolean() { JSON_ASSERT(m_Type == ElementType::Boolean); return *((Boolean*)this); }
        const Boolean& AsBoolean() const { JSON_ASSERT(m_Type == ElementType::Boolean); return *((Boolean*)this); }

        Null& AsNull() { JSON_ASSERT(m_Type == ElementType::Null); return *((Null*)this); }
        const Null& AsNull() const { JSON_ASSERT(m_Type == ElementType::Null); return *((Null*)this); }

        bool operator==(ElementType type) const { return m_Type == type; }

        virtual Element& operator=(const Element& other) noexcept { (void) other; JSON_ASSERT(false); return *this; }
        virtual Element& operator=(Element&& other) noexcept { (void) other; JSON_ASSERT(false); return *this; }

        virtual bool Has(const std::string& key) const { (void) key; return false; }
        virtual std::shared_ptr<Element> At(const std::string& key) { (void) key; return nullptr; }
        virtual const std::shared_ptr<Element> At(const std::string& key) const { (void) key; return nullptr; }
        std::shared_ptr<Element> operator[](const std::string& key) { return At(key); }
        const std::shared_ptr<Element> operator[](const std::string& key) const { return At(key); }

        virtual bool Has(size_t index) const { (void) index; return false; }
        virtual std::shared_ptr<Element> At(size_t index) { (void) index; return nullptr; }
        virtual const std::shared_ptr<Element> At(size_t index) const { (void) index; return nullptr; }
        std::shared_ptr<Element> operator[](size_t index) { return At(index); }
        const std::shared_ptr<Element> operator[](size_t index) const { return At(index); }

        virtual std::string ToString() const { return ElementTypeToString(m_Type); }
        virtual double ToDouble() const { return 0.0; }
        virtual int64_t ToInt64() const { return 0; }
        virtual bool ToBool() const { return false; }

        operator std::string() const { return ToString(); }
        operator double() const { return ToDouble(); }
        operator int64_t() const { return ToInt64(); }
        operator bool() const { return ToBool(); }

        virtual std::string Serialize(bool pretty = false, size_t indent = 2, char indentChar = ' ', size_t maxDepth = 255, size_t _depth = 0) const
        {
            (void) pretty; (void) indent; (void) indentChar; (void) maxDepth; (void) _depth;
            return ToString();
        };

        static Result Parse(ParsingContext& ctx, std::shared_ptr<Element>& out, bool allowDuplicateKeys = false, size_t maxDepth = 255, size_t _depth = 0);
    protected:
        Element(ElementType type)
            : m_Type(type) { }

        ElementType m_Type;
    };

    class String :
        public Element
    {
    public:
        std::string Value;

    public:
        String(const std::string& str)
            : Element(ElementType::String), Value(str) { }

        String(const char* str)
            : String(std::string(str)) { }

        String(const std::u32string& str)
            : String(EncodeUTF8(str)) { }

        String(const char32_t* str)
            : String(EncodeUTF8(str)) { }

        String()
            : String("") { }

        virtual ~String() = default;

        virtual std::string ToString() const override;
        virtual std::string Serialize(bool pretty = false, size_t indent = 2, char indentChar = ' ', size_t maxDepth = 255, size_t _depth = 0) const override;
        static Result Parse(ParsingContext& ctx, String& out);
    };

    class Number :
        public Element
    {
    public:
        Number(double real)
            : Element(ElementType::Number)
        {
            SetReal(real);
        }

        Number(int64_t integer)
            : Element(ElementType::Number)
        {
            SetInteger(integer);
        }

        Number()
            : Number((int64_t)0) { }

        virtual ~Number() = default;
    
        double GetValue() const { return m_IsReal ? m_Real : m_Integer; }
        
        double GetReal() const { return m_Real; }
        int64_t GetInteger() const { return m_Integer; }

        Number& SetReal(double real) { m_IsReal = true; m_Real = real; return *this; }
        Number& SetInteger(int64_t integer) { m_IsReal = false; m_Integer = integer; return *this; }

        virtual std::string ToString() const override { return m_IsReal ? std::to_string(m_Real) : std::to_string(m_Integer); }
        virtual double ToDouble() const override { return (double)(m_IsReal ? m_Real : m_Integer); }
        virtual int64_t ToInt64() const override { return (int64_t)(m_IsReal ? m_Real : m_Integer); }

        virtual std::string Serialize(bool pretty = false, size_t indent = 2, char indentChar = ' ', size_t maxDepth = 255, size_t _depth = 0) const override;
        static Result Parse(ParsingContext& ctx, Number& out);
        static Result ParseInteger(ParsingContext& ctx, bool allowSign, int64_t& out, size_t* digits = nullptr);

    private:
        bool m_IsReal;
        union
        {
            double m_Real;
            int64_t m_Integer;
        };
    };

    class Boolean :
        public Element
    {
    public:
        bool Value;
    
    public:
        Boolean(bool value)
            : Element(ElementType::Boolean), Value(value) { }
        
        Boolean()
            : Boolean(false) { }

        virtual ~Boolean() = default;

        virtual std::string ToString() const override { return Value ? "true" : "false"; }
        virtual bool ToBool() const override { return Value; }

        static Result Parse(ParsingContext& ctx, Boolean& out);
    };

    class Null :
        public Element
    {
    public:
        const void* Value;
    
    public:
        Null()
            : Element(ElementType::Null), Value(nullptr) { }

        virtual ~Null() = default;

        virtual std::string ToString() const override { return "null"; }

        static Result Parse(ParsingContext& ctx, Null& out);
    };

    using Object_T = std::unordered_map<std::string, std::shared_ptr<Element>>;
    class Object :
        public Element
    {
    public:
        Object_T Value;
    
    public:
        Object()
            : Element(ElementType::Object) { }

        // Object(const Object_T& value)
        //     : Element(ElementType::Object), Value(value) { }

        virtual ~Object() = default;

        virtual bool Has(const std::string& key) const override { auto it = Value.find(key); return it != Value.end(); }
        virtual std::shared_ptr<Element> At(const std::string& key) override { return Value[key]; }
        virtual const std::shared_ptr<Element> At(const std::string& key) const override
        {
            if (auto it = Value.find(key); it != Value.end())
                return it->second;
            return nullptr;
        }

        virtual std::string ToString() const override { return "Object{" + std::to_string(Value.size()) + "}"; }

        virtual std::string Serialize(bool pretty = false, size_t indent = 2, char indentChar = ' ', size_t maxDepth = 255, size_t _depth = 0) const override;
        static Result Parse(ParsingContext& ctx, Object& out, bool allowDuplicateKeys = false, size_t maxDepth = 255, size_t _depth = 0);
    };

    using Array_T = std::vector<std::shared_ptr<Element>>;
    class Array :
        public Element
    {
    public:
        Array_T Value;
    
    public:
        Array()
            : Element(ElementType::Array) { }

        // Array(const Array_T& value)
        //     : Element(ElementType::Array), Value(value) { }

        virtual bool Has(size_t index) const override { return index < Value.size(); }

        virtual std::shared_ptr<Element> At(size_t index) override
        {
            if (index >= Value.size())
                return nullptr;
            return Value[index];
        }

        virtual const std::shared_ptr<Element> At(size_t index) const override
        {
            if (index >= Value.size())
                return nullptr;
            return Value[index];
        }

        virtual ~Array() = default;

        virtual std::string ToString() const override { return "Array[" + std::to_string(Value.size()) + "]"; }

        virtual std::string Serialize(bool pretty = false, size_t indent = 2, char indentChar = ' ', size_t maxDepth = 255, size_t _depth = 0) const override;
        static Result Parse(ParsingContext& ctx, Array& out, bool allowDuplicateKeys = false, size_t maxDepth = 255, size_t _depth = 0);
    };

    Result Parse(const std::string& json, std::shared_ptr<Element>& out, bool allowDuplicateKeys = false, size_t maxDepth = 255);
}

#endif // _JSON_H
