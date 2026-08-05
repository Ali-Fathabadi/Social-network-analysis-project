#pragma once

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace json {

class Value;
using Array = std::vector<Value>;

class Object {
public:
    std::vector<std::pair<std::string, Value>> items;

    Value& operator[](const std::string& key);
    bool has(const std::string& key) const;
    const Value& at(const std::string& key) const;
};

enum class Type { Null, Bool, Number, String, Array, Object };

class Value {
public:
    Type type = Type::Null;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::shared_ptr<Array> arrVal;
    std::shared_ptr<Object> objVal;

    Value() = default;
    Value(std::nullptr_t) : type(Type::Null) {}
    Value(bool b) : type(Type::Bool), boolVal(b) {}
    Value(int i) : type(Type::Number), numVal(static_cast<double>(i)) {}
    Value(size_t i) : type(Type::Number), numVal(static_cast<double>(i)) {}
    Value(double d) : type(Type::Number), numVal(d) {}
    Value(const char* s) : type(Type::String), strVal(s ? s : "") {}
    Value(const std::string& s) : type(Type::String), strVal(s) {}
    Value(const Array& a) : type(Type::Array), arrVal(std::make_shared<Array>(a)) {}
    Value(const Object& o) : type(Type::Object), objVal(std::make_shared<Object>(o)) {}

    static Value makeArray() {
        Value v;
        v.type = Type::Array;
        v.arrVal = std::make_shared<Array>();
        return v;
    }

    static Value makeObject() {
        Value v;
        v.type = Type::Object;
        v.objVal = std::make_shared<Object>();
        return v;
    }

    void push_back(const Value& v) {
        if (type != Type::Array) {
            type = Type::Array;
            arrVal = std::make_shared<Array>();
        }
        arrVal->push_back(v);
    }

    Value& operator[](const std::string& key) {
        if (type != Type::Object) {
            type = Type::Object;
            objVal = std::make_shared<Object>();
        }
        return (*objVal)[key];
    }

    bool isNull() const { return type == Type::Null; }

    std::string dump() const {
        std::ostringstream os;
        os.imbue(std::locale::classic());
        write(os);
        return os.str();
    }

    static Value parse(const std::string& text) {
        Parser parser(text);
        Value value = parser.parseValue();
        parser.skipWhitespace();
        if (!parser.finished()) {
            throw std::runtime_error("Unexpected trailing data in JSON");
        }
        return value;
    }

private:
    class Parser {
    public:
        explicit Parser(const std::string& input) : s(input) {}

        bool finished() const { return pos == s.size(); }

        void skipWhitespace() {
            while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) {
                ++pos;
            }
        }

        Value parseValue() {
            skipWhitespace();
            if (pos >= s.size()) throw std::runtime_error("Unexpected end of JSON");
            switch (s[pos]) {
                case '{': return parseObject();
                case '[': return parseArray();
                case '"': return Value(parseString());
                case 't': expect("true"); return Value(true);
                case 'f': expect("false"); return Value(false);
                case 'n': expect("null"); return Value(nullptr);
                default: return parseNumber();
            }
        }

    private:
        const std::string& s;
        size_t pos = 0;

        void expect(const char* literal) {
            const std::string value(literal);
            if (s.compare(pos, value.size(), value) != 0) {
                throw std::runtime_error("Invalid JSON literal");
            }
            pos += value.size();
        }

        static int hexValue(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return 10 + c - 'a';
            if (c >= 'A' && c <= 'F') return 10 + c - 'A';
            return -1;
        }

        unsigned parseHex4() {
            if (pos + 4 > s.size()) throw std::runtime_error("Incomplete Unicode escape");
            unsigned code = 0;
            for (int i = 0; i < 4; ++i) {
                int value = hexValue(s[pos++]);
                if (value < 0) throw std::runtime_error("Invalid Unicode escape");
                code = (code << 4U) | static_cast<unsigned>(value);
            }
            return code;
        }

        static void appendUtf8(std::string& out, unsigned codePoint) {
            if (codePoint <= 0x7F) {
                out.push_back(static_cast<char>(codePoint));
            } else if (codePoint <= 0x7FF) {
                out.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
                out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
            } else if (codePoint <= 0xFFFF) {
                out.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
                out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
            } else if (codePoint <= 0x10FFFF) {
                out.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
                out.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
            } else {
                throw std::runtime_error("Unicode code point out of range");
            }
        }

        std::string parseString() {
            if (pos >= s.size() || s[pos] != '"') {
                throw std::runtime_error("Expected JSON string");
            }
            ++pos;
            std::string out;
            while (pos < s.size()) {
                unsigned char current = static_cast<unsigned char>(s[pos++]);
                if (current == '"') return out;
                if (current < 0x20) throw std::runtime_error("Unescaped control character in string");
                if (current != '\\') {
                    out.push_back(static_cast<char>(current));
                    continue;
                }
                if (pos >= s.size()) throw std::runtime_error("Incomplete escape sequence");
                char escaped = s[pos++];
                switch (escaped) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u': {
                        unsigned first = parseHex4();
                        unsigned codePoint = first;
                        if (first >= 0xD800 && first <= 0xDBFF) {
                            if (pos + 2 > s.size() || s[pos] != '\\' || s[pos + 1] != 'u') {
                                throw std::runtime_error("Missing low surrogate");
                            }
                            pos += 2;
                            unsigned second = parseHex4();
                            if (second < 0xDC00 || second > 0xDFFF) {
                                throw std::runtime_error("Invalid low surrogate");
                            }
                            codePoint = 0x10000 + ((first - 0xD800) << 10U) + (second - 0xDC00);
                        } else if (first >= 0xDC00 && first <= 0xDFFF) {
                            throw std::runtime_error("Unexpected low surrogate");
                        }
                        appendUtf8(out, codePoint);
                        break;
                    }
                    default: throw std::runtime_error("Invalid JSON escape sequence");
                }
            }
            throw std::runtime_error("Unterminated JSON string");
        }

        Value parseNumber() {
            const size_t start = pos;
            if (s[pos] == '-') ++pos;
            if (pos >= s.size()) throw std::runtime_error("Invalid JSON number");

            if (s[pos] == '0') {
                ++pos;
                if (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
                    throw std::runtime_error("Leading zero in JSON number");
                }
            } else if (s[pos] >= '1' && s[pos] <= '9') {
                while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
            } else {
                throw std::runtime_error("Invalid JSON number");
            }

            if (pos < s.size() && s[pos] == '.') {
                ++pos;
                if (pos >= s.size() || !std::isdigit(static_cast<unsigned char>(s[pos]))) {
                    throw std::runtime_error("Invalid fractional JSON number");
                }
                while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
            }

            if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
                ++pos;
                if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
                if (pos >= s.size() || !std::isdigit(static_cast<unsigned char>(s[pos]))) {
                    throw std::runtime_error("Invalid JSON exponent");
                }
                while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
            }

            const std::string number = s.substr(start, pos - start);
            char* end = nullptr;
            const double parsed = std::strtod(number.c_str(), &end);
            if (!end || *end != '\0' || !std::isfinite(parsed)) {
                throw std::runtime_error("JSON number is out of range");
            }
            return Value(parsed);
        }

        Value parseArray() {
            Value result = Value::makeArray();
            ++pos;
            skipWhitespace();
            if (pos < s.size() && s[pos] == ']') {
                ++pos;
                return result;
            }

            while (true) {
                result.arrVal->push_back(parseValue());
                skipWhitespace();
                if (pos >= s.size()) throw std::runtime_error("Unterminated JSON array");
                if (s[pos] == ']') {
                    ++pos;
                    return result;
                }
                if (s[pos] != ',') throw std::runtime_error("Expected comma in JSON array");
                ++pos;
                skipWhitespace();
                if (pos < s.size() && s[pos] == ']') throw std::runtime_error("Trailing comma in JSON array");
            }
        }

        Value parseObject() {
            Value result = Value::makeObject();
            ++pos;
            skipWhitespace();
            if (pos < s.size() && s[pos] == '}') {
                ++pos;
                return result;
            }

            while (true) {
                skipWhitespace();
                std::string key = parseString();
                if (result.objVal->has(key)) throw std::runtime_error("Duplicate key in JSON object");
                skipWhitespace();
                if (pos >= s.size() || s[pos] != ':') throw std::runtime_error("Expected colon in JSON object");
                ++pos;
                result.objVal->items.push_back({key, parseValue()});
                skipWhitespace();
                if (pos >= s.size()) throw std::runtime_error("Unterminated JSON object");
                if (s[pos] == '}') {
                    ++pos;
                    return result;
                }
                if (s[pos] != ',') throw std::runtime_error("Expected comma in JSON object");
                ++pos;
                skipWhitespace();
                if (pos < s.size() && s[pos] == '}') throw std::runtime_error("Trailing comma in JSON object");
            }
        }
    };

    void write(std::ostringstream& os) const {
        switch (type) {
            case Type::Null: os << "null"; break;
            case Type::Bool: os << (boolVal ? "true" : "false"); break;
            case Type::Number:
                if (!std::isfinite(numVal)) {
                    os << "null";
                } else if (std::floor(numVal) == numVal && std::abs(numVal) < 1e15) {
                    os << static_cast<long long>(numVal);
                } else {
                    os << std::setprecision(15) << numVal;
                }
                break;
            case Type::String: writeString(os, strVal); break;
            case Type::Array:
                os << '[';
                for (size_t i = 0; i < arrVal->size(); ++i) {
                    if (i) os << ',';
                    (*arrVal)[i].write(os);
                }
                os << ']';
                break;
            case Type::Object:
                os << '{';
                for (size_t i = 0; i < objVal->items.size(); ++i) {
                    if (i) os << ',';
                    writeString(os, objVal->items[i].first);
                    os << ':';
                    objVal->items[i].second.write(os);
                }
                os << '}';
                break;
        }
    }

    static void writeString(std::ostringstream& os, const std::string& value) {
        os << '"';
        for (unsigned char c : value) {
            switch (c) {
                case '"': os << "\\\""; break;
                case '\\': os << "\\\\"; break;
                case '\b': os << "\\b"; break;
                case '\f': os << "\\f"; break;
                case '\n': os << "\\n"; break;
                case '\r': os << "\\r"; break;
                case '\t': os << "\\t"; break;
                default:
                    if (c < 0x20) {
                        char buffer[7];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(c));
                        os << buffer;
                    } else {
                        os << static_cast<char>(c);
                    }
            }
        }
        os << '"';
    }
};

inline Value& Object::operator[](const std::string& key) {
    for (auto& item : items) {
        if (item.first == key) return item.second;
    }
    items.push_back({key, Value()});
    return items.back().second;
}

inline bool Object::has(const std::string& key) const {
    for (const auto& item : items) {
        if (item.first == key) return true;
    }
    return false;
}

inline const Value& Object::at(const std::string& key) const {
    for (const auto& item : items) {
        if (item.first == key) return item.second;
    }
    throw std::runtime_error("Key not found: " + key);
}

}
