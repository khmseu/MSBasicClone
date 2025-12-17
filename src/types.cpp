#include "types.h"
#include "float40.h"
#include <stdexcept>
#include <sstream>

Value::Value() : data(0.0) {}

Value::Value(double num) : data(num) {}

Value::Value(const std::string& str) : data(str) {}

Value::Value(const Float40& f40) : data(f40.toDouble()) {}

bool Value::isNumber() const {
    return std::holds_alternative<double>(data);
}

bool Value::isString() const {
    return std::holds_alternative<std::string>(data);
}

double Value::getNumber() const {
    if (isNumber()) {
        return std::get<double>(data);
    }
    if (isString()) {
        // Try to convert string to number
        const std::string& str = std::get<std::string>(data);
        try {
            return std::stod(str);
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

std::string Value::getString() const {
    if (isString()) {
        return std::get<std::string>(data);
    }
    if (isNumber()) {
        // Convert number to string
        Float40 f(std::get<double>(data));
        return f.toString();
    }
    return "";
}

Value Value::operator+(const Value& other) const {
    if (isString() || other.isString()) {
        // String concatenation
        return Value(getString() + other.getString());
    }
    // Numeric addition
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a + b).toDouble());
}

Value Value::operator-(const Value& other) const {
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a - b).toDouble());
}

Value Value::operator*(const Value& other) const {
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a * b).toDouble());
}

Value Value::operator/(const Value& other) const {
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a / b).toDouble());
}

bool Value::operator==(const Value& other) const {
    if (isString() && other.isString()) {
        return getString() == other.getString();
    }
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return a == b;
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

bool Value::operator<(const Value& other) const {
    if (isString() && other.isString()) {
        return getString() < other.getString();
    }
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return a < b;
}

bool Value::operator>(const Value& other) const {
    if (isString() && other.isString()) {
        return getString() > other.getString();
    }
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return a > b;
}

bool Value::operator<=(const Value& other) const {
    return !(*this > other);
}

bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}
