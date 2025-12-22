/**
 * @file float40.h
 * @brief Applesoft-compatible 40-bit floating point number implementation
 * 
 * The Float40 class implements the 40-bit (5-byte) floating point format
 * used by Applesoft BASIC on the Apple II. While this implementation uses
 * doubles internally for computation, it maintains precision characteristics
 * matching Applesoft's format.
 * 
 * Format details:
 * - 1 byte: Exponent (biased)
 * - 4 bytes: Mantissa (normalized)
 * - Precision: Approximately 9-10 significant decimal digits
 * - Range: Similar to Applesoft (approximately ±10^±38)
 * 
 * The Float40 class ensures that calculations match Applesoft behavior,
 * including precision limits and rounding. This is important for programs
 * that depend on specific numeric behavior.
 * 
 * Key differences from IEEE 754 double:
 * - Lower precision (9-10 digits vs. 15-16 digits)
 * - Different rounding behavior at precision boundaries
 * - Matches Applesoft numeric output format
 */

#pragma once

#include <cstdint>
#include <string>

/**
 * @class Float40
 * @brief Applesoft-compatible 40-bit floating point number
 * 
 * Implements the 5-byte floating point format used by Applesoft BASIC,
 * providing arithmetic operations and math functions with precision
 * matching the original Apple II implementation.
 * 
 * Usage:
 * @code
 * Float40 a(3.14159);
 * Float40 b("2.71828");
 * Float40 result = a * b;
 * std::cout << result.toString() << std::endl;
 * @endcode
 */
class Float40 {
public:
  /**
   * @brief Default constructor, initializes to zero
   */
  Float40();
  
  /**
   * @brief Construct from double value
   * @param value Initial value
   */
  explicit Float40(double value);
  
  /**
   * @brief Construct from integer value
   * @param value Initial value
   */
  explicit Float40(int value);
  
  /**
   * @brief Construct from string representation
   * @param str Numeric string (e.g., "3.14", "1.5E10")
   */
  explicit Float40(const std::string &str);

  // Conversion
  
  /**
   * @brief Convert to standard double
   * @return Double precision value
   */
  double toDouble() const;
  
  /**
   * @brief Convert to Applesoft-style string
   * @return String representation matching Applesoft output format
   */
  std::string toString() const;

  // Arithmetic operations
  
  /**
   * @brief Addition operator
   * @param other Right operand
   * @return Sum with Applesoft precision
   */
  Float40 operator+(const Float40 &other) const;
  
  /**
   * @brief Subtraction operator
   * @param other Right operand
   * @return Difference with Applesoft precision
   */
  Float40 operator-(const Float40 &other) const;
  
  /**
   * @brief Multiplication operator
   * @param other Right operand
   * @return Product with Applesoft precision
   */
  Float40 operator*(const Float40 &other) const;
  
  /**
   * @brief Division operator
   * @param other Right operand
   * @return Quotient with Applesoft precision
   * @throws RuntimeError on division by zero
   */
  Float40 operator/(const Float40 &other) const;
  
  /**
   * @brief Unary negation operator
   * @return Negated value
   */
  Float40 operator-() const;

  // Comparison operations
  
  /**
   * @brief Equality comparison
   * @param other Right operand
   * @return true if values are equal within precision
   */
  bool operator==(const Float40 &other) const;
  
  /**
   * @brief Inequality comparison
   * @param other Right operand
   * @return true if values are not equal
   */
  bool operator!=(const Float40 &other) const;
  
  /**
   * @brief Less than comparison
   * @param other Right operand
   * @return true if this < other
   */
  bool operator<(const Float40 &other) const;
  
  /**
   * @brief Greater than comparison
   * @param other Right operand
   * @return true if this > other
   */
  bool operator>(const Float40 &other) const;
  
  /**
   * @brief Less than or equal comparison
   * @param other Right operand
   * @return true if this <= other
   */
  bool operator<=(const Float40 &other) const;
  
  /**
   * @brief Greater than or equal comparison
   * @param other Right operand
   * @return true if this >= other
   */
  bool operator>=(const Float40 &other) const;

  // Math functions
  
  /**
   * @brief Exponentiation (^)
   * @param exponent Power to raise to
   * @return This value raised to exponent power
   */
  Float40 power(const Float40 &exponent) const;
  
  /**
   * @brief Modulo operation (MOD)
   * @param divisor Divisor
   * @return Remainder after division
   */
  Float40 mod(const Float40 &divisor) const;
  
  /**
   * @brief Absolute value (ABS)
   * @return Absolute value
   */
  Float40 abs() const;
  
  /**
   * @brief Sign function (SGN)
   * @return -1, 0, or 1 based on sign
   */
  Float40 sgn() const;
  
  /**
   * @brief Integer part (INT)
   * @return Largest integer not greater than value (floor)
   */
  Float40 intPart() const;

  // Trigonometric functions
  
  /**
   * @brief Sine function (SIN)
   * @return Sine of value (value in radians)
   */
  Float40 sin() const;
  
  /**
   * @brief Cosine function (COS)
   * @return Cosine of value (value in radians)
   */
  Float40 cos() const;
  
  /**
   * @brief Tangent function (TAN)
   * @return Tangent of value (value in radians)
   */
  Float40 tan() const;
  
  /**
   * @brief Arctangent function (ATN)
   * @return Arctangent of value in radians
   */
  Float40 atn() const;

  // Exponential and logarithmic
  
  /**
   * @brief Exponential function (EXP)
   * @return e raised to this value
   */
  Float40 exp() const;
  
  /**
   * @brief Natural logarithm (LOG)
   * @return Natural logarithm of value
   * @throws RuntimeError if value <= 0
   */
  Float40 log() const;
  
  /**
   * @brief Square root (SQR)
   * @return Square root of value
   * @throws RuntimeError if value < 0
   */
  Float40 sqr() const;

  // Random number (static)
  
  /**
   * @brief Generate random number (RND)
   * @param seed Seed control (see funcRnd documentation)
   * @return Random Float40 in range [0, 1)
   */
  static Float40 rnd(const Float40 &seed);
  
  /**
   * @brief Set random number seed
   * @param seed New seed value
   */
  static void setSeed(const Float40 &seed);

private:
  double value_;

  // Internal precision matching Applesoft (approximately 9-10 digits)
  static constexpr double PRECISION_FACTOR = 1e9;
  
  /**
   * @brief Round value to Applesoft precision
   * @return Value rounded to match Applesoft precision limits
   */
  double roundToPrecision() const;
};
