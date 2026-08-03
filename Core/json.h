// json.h - Minimal dependency-free JSON library for this project.
// Supports: null, bool, number (as double, printed as int when integral),
// string, array, object (insertion-ordered).
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cmath>

namespace json {

class Value;
using Array = std::vector<Value>;

// Ordered key-value list (preserves insertion order, unlike std::map)
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
    double numVal = 0;
    std::string strVal;
    std::shared_ptr<Array> arrVal;
    std::shared_ptr<Object> objVal;

    Value() : type(Type::Null) {}
    Value(std::nullptr_t) : type(Type::Null) {}
    Value(bool b) : type(Type::Bool), boolVal(b) {}
    Value(int i) : type(Type::Number), numVal(i) {}
    Value(size_t i) : type(Type::Number), numVal((double)i) {}
    Value(double d) : type(Type::Number), numVal(d) {}
    Value(const char* s) : type(Type::String), strVal(s) {}
    Value(const std::string& s) : type(Type::String), strVal(s) {}
    Value(const Array& a) : type(Type::Array), arrVal(std::make_shared<Array>(a)) {}
    Value(const Object& o) : type(Type::Object), objVal(std::make_shared<Object>(o)) {}

    static Value makeArray() { Value v; v.type = Type::Array; v.arrVal = std::make_shared<Array>(); return v; }
    static Value makeObject() { Value v; v.type = Type::Object; v.objVal = std::make_shared<Object>(); return v; }

    void push_back(const Value& v) {
        if (type != Type::Array) { type = Type::Array; arrVal = std::make_shared<Array>(); }
        arrVal->push_back(v);
    }

    Value& operator[](const std::string& key) {
        if (type != Type::Object) { type = Type::Object; objVal = std::make_shared<Object>(); }
        return (*objVal)[key];
    }

    bool isNull() const { return type == Type::Null; }

    // ---------- Serialization ----------
    std::string dump() const {
        std::ostringstream os;
        write(os);
        return os.str();
    }

    void write(std::ostringstream& os) const {
        switch (type) {
            case Type::Null: os << "null"; break;
            case Type::Bool: os << (boolVal ? "true" : "false"); break;
            case Type::Number: {
                double d = numVal;
                if (std::floor(d) == d && std::abs(d) < 1e15) {
                    os << (long long)d;
                } else {
                    os << d;
                }
                break;
            }
            case Type::String: writeString(os, strVal); break;
            case Type::Array: {
                os << "[";
                for (size_t i = 0; i < arrVal->size(); ++i) {
                    if (i) os << ",";
                    (*arrVal)[i].write(os);
                }
                os << "]";
                break;
            }
            case Type::Object: {
                os << "{";
                for (size_t i = 0; i < objVal->items.size(); ++i) {
                    if (i) os << ",";
                    writeString(os, objVal->items[i].first);
                    os << ":";
                    objVal->items[i].second.write(os);
                }
                os << "}";
                break;
            }
        }
    }

    static void writeString(std::ostringstream& os, const std::string& s) {
        os << "\"";
        for (char c : s) {
            switch (c) {
                case '"': os << "\\\""; break;
                case '\\': os << "\\\\"; break;
                case '\n': os << "\\n"; break;
                case '\t': os << "\\t"; break;
                case '\r': os << "\\r"; break;
                default:
                    if ((unsigned char)c < 0x20) {
                        char buf[8];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        os << buf;
                    } else {
                        os << c;
                    }
            }
        }
        os << "\"";
    }

    // ---------- Parsing ----------
    static Value parse(const std::string& s) {
        size_t pos = 0;
        skipWs(s, pos);
        Value v = parseValue(s, pos);
        return v;
    }

private:
    static void skipWs(const std::string& s, size_t& pos) {
        while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
    }

    static Value parseValue(const std::string& s, size_t& pos) {
        skipWs(s, pos);
        if (pos >= s.size()) throw std::runtime_error("Unexpected end of JSON");
        char c = s[pos];
        if (c == '{') return parseObject(s, pos);
        if (c == '[') return parseArray(s, pos);
        if (c == '"') return Value(parseString(s, pos));
        if (c == 't') { expect(s, pos, "true"); return Value(true); }
        if (c == 'f') { expect(s, pos, "false"); return Value(false); }
        if (c == 'n') { expect(s, pos, "null"); return Value(nullptr); }
        return parseNumber(s, pos);
    }

    static void expect(const std::string& s, size_t& pos, const std::string& lit) {
        if (s.compare(pos, lit.size(), lit) != 0) throw std::runtime_error("Invalid literal in JSON");
        pos += lit.size();
    }

    static std::string parseString(const std::string& s, size_t& pos) {
        std::string out;
        pos++; // skip opening quote
        while (pos < s.size() && s[pos] != '"') {
            char c = s[pos];
            if (c == '\\') {
                pos++;
                if (pos >= s.size()) break;
                char e = s[pos];
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case 'u': {
                        if (pos + 4 < s.size()) {
                            std::string hex = s.substr(pos + 1, 4);
                            int code = std::stoi(hex, nullptr, 16);
                            // Basic BMP handling; encode as UTF-8
                            if (code < 0x80) out += (char)code;
                            else if (code < 0x800) {
                                out += (char)(0xC0 | (code >> 6));
                                out += (char)(0x80 | (code & 0x3F));
                            } else {
                                out += (char)(0xE0 | (code >> 12));
                                out += (char)(0x80 | ((code >> 6) & 0x3F));
                                out += (char)(0x80 | (code & 0x3F));
                            }
                            pos += 4;
                        }
                        break;
                    }
                    default: out += e;
                }
                pos++;
            } else {
                out += c;
                pos++;
            }
        }
        pos++; // skip closing quote
        return out;
    }

    static Value parseNumber(const std::string& s, size_t& pos) {
        size_t start = pos;
        if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) pos++;
        while (pos < s.size() && (std::isdigit((unsigned char)s[pos]) || s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+' || s[pos] == '-')) pos++;
        std::string numStr = s.substr(start, pos - start);
        return Value(std::stod(numStr));
    }

    static Value parseArray(const std::string& s, size_t& pos) {
        Value v = Value::makeArray();
        pos++; // skip [
        skipWs(s, pos);
        if (pos < s.size() && s[pos] == ']') { pos++; return v; }
        while (true) {
            Value item = parseValue(s, pos);
            v.arrVal->push_back(item);
            skipWs(s, pos);
            if (pos < s.size() && s[pos] == ',') { pos++; continue; }
            break;
        }
        skipWs(s, pos);
        if (pos < s.size() && s[pos] == ']') pos++;
        return v;
    }

    static Value parseObject(const std::string& s, size_t& pos) {
        Value v = Value::makeObject();
        pos++; // skip {
        skipWs(s, pos);
        if (pos < s.size() && s[pos] == '}') { pos++; return v; }
        while (true) {
            skipWs(s, pos);
            std::string key = parseString(s, pos);
            skipWs(s, pos);
            if (pos < s.size() && s[pos] == ':') pos++;
            Value val = parseValue(s, pos);
            v.objVal->items.push_back({key, val});
            skipWs(s, pos);
            if (pos < s.size() && s[pos] == ',') { pos++; continue; }
            break;
        }
        skipWs(s, pos);
        if (pos < s.size() && s[pos] == '}') pos++;
        return v;
    }
};

inline Value& Object::operator[](const std::string& key) {
    for (auto& p : items) if (p.first == key) return p.second;
    items.push_back({key, Value()});
    return items.back().second;
}

inline bool Object::has(const std::string& key) const {
    for (auto& p : items) if (p.first == key) return true;
    return false;
}

inline const Value& Object::at(const std::string& key) const {
    for (auto& p : items) if (p.first == key) return p.second;
    throw std::runtime_error("Key not found: " + key);
}

} // namespace json
