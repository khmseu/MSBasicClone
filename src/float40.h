#pragma once

#include <cstdint>
#include <string>

// Applesoft BASIC uses 40-bit floating point (5 bytes)
// Format: 1 byte exponent + 4 bytes mantissa
// This is a simplified implementation using doubles internally
// but maintaining the same precision characteristics

class Float40 {
public:
  Float40();
  explicit Float40(double value);
  explicit Float40(int value);
  explicit Float40(const std::string &str);

  // Conversion
  double toDouble() const;
  std::string toString() const;

  // Arithmetic operations
  Float40 operator+(const Float40 &other) const;
  Float40 operator-(const Float40 &other) const;
  Float40 operator*(const Float40 &other) const;
  Float40 operator/(const Float40 &other) const;
  Float40 operator-() const;

  // Comparison operations
  bool operator==(const Float40 &other) const;
  bool operator!=(const Float40 &other) const;
  bool operator<(const Float40 &other) const;
  bool operator>(const Float40 &other) const;
  bool operator<=(const Float40 &other) const;
  bool operator>=(const Float40 &other) const;

  // Math functions
  Float40 power(const Float40 &exponent) const;
  Float40 mod(const Float40 &divisor) const;
  Float40 abs() const;
  Float40 sgn() const;
  Float40 intPart() const;

  // Trigonometric functions
  Float40 sin() const;
  Float40 cos() const;
  Float40 tan() const;
  Float40 atn() const;

  // Exponential and logarithmic
  Float40 exp() const;
  Float40 log() const;
  Float40 sqr() const;

  // Random number (static)
  static Float40 rnd(const Float40 &seed);
  static void setSeed(const Float40 &seed);

private:
  double value_;

  // Internal precision matching Applesoft (approximately 9-10 digits)
  static constexpr double PRECISION_FACTOR = 1e9;
  double roundToPrecision() const;
};
