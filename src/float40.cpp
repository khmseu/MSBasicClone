#include "float40.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <random>

Float40::Float40() : value_(0.0) {}

Float40::Float40(double value) : value_(value) {
    value_ = roundToPrecision();
}

Float40::Float40(int value) : value_(static_cast<double>(value)) {}

Float40::Float40(const std::string& str) {
    try {
        value_ = std::stod(str);
        value_ = roundToPrecision();
    } catch (...) {
        value_ = 0.0;
    }
}

double Float40::toDouble() const {
    return value_;
}

std::string Float40::toString() const {
    std::ostringstream oss;
    
    // Match Applesoft output format
    if (std::abs(value_) >= 1e9 || (std::abs(value_) < 0.01 && value_ != 0.0)) {
        // Scientific notation
        oss << std::scientific << std::setprecision(8) << value_;
    } else {
        // Fixed notation
        oss << std::fixed << std::setprecision(9) << value_;
        std::string result = oss.str();
        
        // Remove trailing zeros
        size_t dot = result.find('.');
        if (dot != std::string::npos) {
            size_t end = result.find_last_not_of('0');
            if (end > dot) {
                result = result.substr(0, end + 1);
            }
            if (result.back() == '.') {
                result.pop_back();
            }
        }
        return result;
    }
    
    return oss.str();
}

double Float40::roundToPrecision() const {
    if (value_ == 0.0) return 0.0;
    
    // Round to approximately 9 significant digits (Applesoft precision)
    double scale = std::pow(10.0, 9 - std::ceil(std::log10(std::abs(value_))));
    return std::round(value_ * scale) / scale;
}

Float40 Float40::operator+(const Float40& other) const {
    return Float40(value_ + other.value_);
}

Float40 Float40::operator-(const Float40& other) const {
    return Float40(value_ - other.value_);
}

Float40 Float40::operator*(const Float40& other) const {
    return Float40(value_ * other.value_);
}

Float40 Float40::operator/(const Float40& other) const {
    if (other.value_ == 0.0) {
        throw std::runtime_error("DIVISION BY ZERO ERROR");
    }
    return Float40(value_ / other.value_);
}

Float40 Float40::operator-() const {
    return Float40(-value_);
}

bool Float40::operator==(const Float40& other) const {
    return std::abs(value_ - other.value_) < 1e-9;
}

bool Float40::operator!=(const Float40& other) const {
    return !(*this == other);
}

bool Float40::operator<(const Float40& other) const {
    return value_ < other.value_ - 1e-9;
}

bool Float40::operator>(const Float40& other) const {
    return value_ > other.value_ + 1e-9;
}

bool Float40::operator<=(const Float40& other) const {
    return !(*this > other);
}

bool Float40::operator>=(const Float40& other) const {
    return !(*this < other);
}

Float40 Float40::power(const Float40& exponent) const {
    return Float40(std::pow(value_, exponent.value_));
}

Float40 Float40::mod(const Float40& divisor) const {
    return Float40(std::fmod(value_, divisor.value_));
}

Float40 Float40::abs() const {
    return Float40(std::abs(value_));
}

Float40 Float40::sgn() const {
    if (value_ > 0) return Float40(1.0);
    if (value_ < 0) return Float40(-1.0);
    return Float40(0.0);
}

Float40 Float40::intPart() const {
    return Float40(std::floor(value_));
}

Float40 Float40::sin() const {
    return Float40(std::sin(value_));
}

Float40 Float40::cos() const {
    return Float40(std::cos(value_));
}

Float40 Float40::tan() const {
    return Float40(std::tan(value_));
}

Float40 Float40::atn() const {
    return Float40(std::atan(value_));
}

Float40 Float40::exp() const {
    return Float40(std::exp(value_));
}

Float40 Float40::log() const {
    if (value_ <= 0) {
        throw std::runtime_error("ILLEGAL QUANTITY ERROR");
    }
    return Float40(std::log(value_));
}

Float40 Float40::sqr() const {
    if (value_ < 0) {
        throw std::runtime_error("ILLEGAL QUANTITY ERROR");
    }
    return Float40(std::sqrt(value_));
}

Float40 Float40::rnd(const Float40& seed) {
    static std::mt19937 generator;
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    
    if (seed.value_ < 0) {
        // Negative seed: reseed the generator
        generator.seed(static_cast<unsigned int>(-seed.value_));
    } else if (seed.value_ == 0) {
        // Zero: repeat last random number (simplified - just return new one)
        return Float40(distribution(generator));
    }
    // Positive or zero: generate new random number
    return Float40(distribution(generator));
}
