/**
 * @file types.cpp
 * @brief Implementation of Value class and core type operations
 * 
 * This file implements the Value class which represents runtime values in
 * MSBasic. A Value can hold either a numeric value (double) or a string,
 * using std::variant for type-safe storage.
 * 
 * Key features:
 * - Automatic type coercion (string to number, number to string)
 * - Operator overloading for arithmetic and comparison operations
 * - Integration with Float40 for Applesoft-compatible floating point
 * - String concatenation when either operand is a string (+ operator)
 * 
 * All numeric operations use the Float40 class to maintain Applesoft BASIC's
 * 40-bit floating-point precision and behavior.
 */

#include "types.h"
#include "float40.h"
#include <stdexcept>
#include <sstream>

// Constructors

/**
 * @brief Construct a default Value (numeric 0.0)
 */
Value::Value() : data(0.0) {}

/**
 * @brief Construct a numeric Value
 * @param num Numeric value to store
 */
Value::Value(double num) : data(num) {}

/**
 * @brief Construct a string Value
 * @param str String value to store
 */
Value::Value(const std::string& str) : data(str) {}

/**
 * @brief Construct a Value from Float40
 * @param f40 Float40 value (converted to double)
 */
Value::Value(const Float40& f40) : data(f40.toDouble()) {}

// Type checking

/**
 * @brief Check if this Value holds a number
 * @return true if value is numeric, false otherwise
 */
bool Value::isNumber() const {
    return std::holds_alternative<double>(data);
}

/**
 * @brief Check if this Value holds a string
 * @return true if value is a string, false otherwise
 */
bool Value::isString() const {
    return std::holds_alternative<std::string>(data);
}

// Type conversion

/**
 * @brief Get value as a number
 * 
 * If the value is already numeric, returns it directly.
 * If the value is a string, attempts to parse it as a number using std::stod.
 * Returns 0.0 if conversion fails or string is invalid.
 * 
 * @return Numeric representation of the value
 */
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

/**
 * @brief Get value as a string
 * 
 * If the value is already a string, returns it directly.
 * If the value is numeric, converts it using Float40::toString() to
 * maintain Applesoft formatting conventions.
 * 
 * @return String representation of the value
 */
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

// Arithmetic operators

/**
 * @brief Addition/concatenation operator
 * 
 * If either operand is a string, performs string concatenation.
 * Otherwise, performs numeric addition using Float40 arithmetic.
 * 
 * @param other Right-hand operand
 * @return Result of addition or concatenation
 */
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

/**
 * @brief Subtraction operator
 * 
 * Performs numeric subtraction using Float40 arithmetic.
 * Both operands are coerced to numbers if necessary.
 * 
 * @param other Right-hand operand
 * @return Result of subtraction
 */
Value Value::operator-(const Value& other) const {
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a - b).toDouble());
}

/**
 * @brief Multiplication operator
 * 
 * Performs numeric multiplication using Float40 arithmetic.
 * Both operands are coerced to numbers if necessary.
 * 
 * @param other Right-hand operand
 * @return Result of multiplication
 */
Value Value::operator*(const Value& other) const {
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a * b).toDouble());
}

/**
 * @brief Division operator
 * 
 * Performs numeric division using Float40 arithmetic.
 * Both operands are coerced to numbers if necessary.
 * Division by zero behavior follows Float40 conventions.
 * 
 * @param other Right-hand operand
 * @return Result of division
 */
Value Value::operator/(const Value& other) const {
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return Value((a / b).toDouble());
}

// Comparison operators

/**
 * @brief Equality comparison
 * 
 * For strings: compares strings lexicographically
 * For numbers: compares using Float40 equality
 * Mixed types are coerced to numbers for comparison
 * 
 * @param other Right-hand operand
 * @return true if values are equal, false otherwise
 */
bool Value::operator==(const Value& other) const {
    if (isString() && other.isString()) {
        return getString() == other.getString();
    }
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return a == b;
}

/**
 * @brief Inequality comparison
 * @param other Right-hand operand
 * @return true if values are not equal, false otherwise
 */
bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

/**
 * @brief Less-than comparison
 * 
 * For strings: compares strings lexicographically
 * For numbers: compares using Float40 comparison
 * Mixed types are coerced to numbers for comparison
 * 
 * @param other Right-hand operand
 * @return true if this value is less than other, false otherwise
 */
bool Value::operator<(const Value& other) const {
    if (isString() && other.isString()) {
        return getString() < other.getString();
    }
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return a < b;
}

/**
 * @brief Greater-than comparison
 * 
 * For strings: compares strings lexicographically
 * For numbers: compares using Float40 comparison
 * Mixed types are coerced to numbers for comparison
 * 
 * @param other Right-hand operand
 * @return true if this value is greater than other, false otherwise
 */
bool Value::operator>(const Value& other) const {
    if (isString() && other.isString()) {
        return getString() > other.getString();
    }
    Float40 a(getNumber());
    Float40 b(other.getNumber());
    return a > b;
}

/**
 * @brief Less-than-or-equal comparison
 * @param other Right-hand operand
 * @return true if this value is less than or equal to other, false otherwise
 */
bool Value::operator<=(const Value& other) const {
    return !(*this > other);
}

/**
 * @brief Greater-than-or-equal comparison
 * @param other Right-hand operand
 * @return true if this value is greater than or equal to other, false otherwise
 */
bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}
