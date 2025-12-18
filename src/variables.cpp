#include "variables.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

Variables::Variables() {}

std::string Variables::normalizeName(const std::string &name) const {
  // In Applesoft BASIC, only first 2 characters are significant
  std::string normalized = name;
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 ::toupper);

  // Preserve the distinguishing character of user-defined functions (FNx)
  if (normalized.rfind("FN", 0) == 0 && normalized.length() > 2) {
    return normalized.substr(0, 3);
  }

  // For non-string, non-integer variables, use first 2 chars
  if (normalized.length() > 2 && normalized.back() != '$' &&
      normalized.back() != '%') {
    return normalized.substr(0, 2);
  }

  return normalized;
}

namespace {
Value coerceInteger(const Value &value) {
  double n = value.getNumber();
  // Applesoft integer variables are 16-bit signed; clamp to range.
  double clamped = std::llround(n);
  if (clamped > 32767.0)
    clamped = 32767.0;
  if (clamped < -32768.0)
    clamped = -32768.0;
  return Value(clamped);
}
} // namespace

void Variables::setVariable(const std::string &name, const Value &value) {
  if (!name.empty() && name.back() == '%') {
    variables_[normalizeName(name)] = coerceInteger(value);
    return;
  }
  variables_[normalizeName(name)] = value;
}

void Variables::unsetVariable(const std::string &name) {
  variables_.erase(normalizeName(name));
}

Value Variables::getVariable(const std::string &name) {
  std::string normalized = normalizeName(name);
  auto it = variables_.find(normalized);
  if (it != variables_.end()) {
    return it->second;
  }

  // Uninitialized variables default to 0 or empty string
  if (!name.empty() && name.back() == '$') {
    return Value("");
  }
  return Value(0.0);
}

bool Variables::hasVariable(const std::string &name) const {
  return variables_.find(normalizeName(name)) != variables_.end();
}

void Variables::clear() {
  variables_.clear();
  arrays_.clear();
  // Don't clear functions - they persist
}

void Variables::dimArray(const std::string &name,
                         const std::vector<int> &dimensions) {
  std::string normalized = normalizeName(name);
  ArrayInfo &arr = arrays_[normalized];
  arr.dimensions = dimensions;
  arr.data.clear();
}

void Variables::setArrayElement(const std::string &name,
                                const std::vector<int> &indices,
                                const Value &value) {
  std::string normalized = normalizeName(name);

  if (arrays_.find(normalized) == arrays_.end()) {
    // Auto-dimension to 10 if not explicitly dimensioned (Applesoft behavior)
    std::vector<int> defaultDims(indices.size(), 10);
    dimArray(name, defaultDims);
  }

  ArrayInfo &arr = arrays_[normalized];

  // Check bounds
  for (size_t i = 0; i < indices.size() && i < arr.dimensions.size(); ++i) {
    if (indices[i] < 0 || indices[i] > arr.dimensions[i]) {
      throw std::runtime_error("BAD SUBSCRIPT ERROR");
    }
  }

  if (!name.empty() && name.back() == '%') {
    arr.data[indices] = coerceInteger(value);
  } else {
    arr.data[indices] = value;
  }
}

Value Variables::getArrayElement(const std::string &name,
                                 const std::vector<int> &indices) {
  std::string normalized = normalizeName(name);

  if (arrays_.find(normalized) == arrays_.end()) {
    // Auto-dimension to 10 if not explicitly dimensioned
    std::vector<int> defaultDims(indices.size(), 10);
    dimArray(name, defaultDims);
  }

  ArrayInfo &arr = arrays_[normalized];

  // Check bounds
  for (size_t i = 0; i < indices.size() && i < arr.dimensions.size(); ++i) {
    if (indices[i] < 0 || indices[i] > arr.dimensions[i]) {
      throw std::runtime_error("BAD SUBSCRIPT ERROR");
    }
  }

  auto it = arr.data.find(indices);
  if (it != arr.data.end()) {
    return it->second;
  }

  // Uninitialized array elements default to 0 or empty string
  if (!name.empty() && name.back() == '$') {
    return Value("");
  }
  return Value(0.0);
}

void Variables::defineFunction(const std::string &name,
                               const std::string &param,
                               const std::shared_ptr<Expression> &expr) {
  std::string normalized = normalizeName(name);
  FunctionInfo &func = functions_[normalized];
  func.parameter = param;
  func.body = expr;
}

bool Variables::hasFunction(const std::string &name) const {
  return functions_.find(normalizeName(name)) != functions_.end();
}

const Variables::FunctionInfo &Variables::getFunction(const std::string &name) {
  std::string normalized = normalizeName(name);
  auto it = functions_.find(normalized);
  if (it == functions_.end()) {
    throw std::runtime_error("UNDEFINED FUNCTION ERROR");
  }
  return it->second;
}
