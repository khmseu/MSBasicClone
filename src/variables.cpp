#include "variables.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
// Applesoft BASIC variable name significance limits
constexpr size_t VARIABLE_NAME_LENGTH = 2;
constexpr size_t FN_FUNCTION_NAME_LENGTH = 4; // FN prefix + 2 chars
} // namespace

Variables::Variables() {}

std::string Variables::normalizeName(const std::string &name) const {
  // In Applesoft BASIC, only first 2 characters are significant
  std::string normalized = name;
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 ::toupper);

  // Preserve two significant characters in the name part of user-defined functions (FNxy)
  if (normalized.rfind("FN", 0) == 0 && normalized.length() > 2) {
    return normalized.substr(0, std::min(normalized.length(), FN_FUNCTION_NAME_LENGTH));
  }

  // For non-string, non-integer variables, use first 2 chars
  if (normalized.length() > VARIABLE_NAME_LENGTH && normalized.back() != '$' &&
      normalized.back() != '%') {
    return normalized.substr(0, VARIABLE_NAME_LENGTH);
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

bool Variables::hasArray(const std::string &name) const {
  return arrays_.find(normalizeName(name)) != arrays_.end();
}

const std::vector<int> &Variables::getArrayDimensions(const std::string &name) const {
  std::string normalized = normalizeName(name);
  auto it = arrays_.find(normalized);
  if (it == arrays_.end()) {
    throw std::runtime_error("UNDEFINED ARRAY ERROR");
  }
  return it->second.dimensions;
}

const std::map<std::vector<int>, Value> &
Variables::getArrayData(const std::string &name) const {
  std::string normalized = normalizeName(name);
  auto it = arrays_.find(normalized);
  if (it == arrays_.end()) {
    throw std::runtime_error("UNDEFINED ARRAY ERROR");
  }
  return it->second.data;
}

void Variables::setArrayData(const std::string &name,
                             const std::vector<int> &dimensions,
                             const std::map<std::vector<int>, Value> &data) {
  std::string normalized = normalizeName(name);
  ArrayInfo &arr = arrays_[normalized];
  arr.dimensions = dimensions;
  arr.data = data;
}

std::map<std::string, double> Variables::getAllNumericVariables() const {
  std::map<std::string, double> numVars;
  for (const auto &pair : variables_) {
    if (!pair.second.isString()) {
      numVars[pair.first] = pair.second.getNumber();
    }
  }
  return numVars;
}

std::map<std::string, std::string> Variables::getAllStringVariables() const {
  std::map<std::string, std::string> strVars;
  for (const auto &pair : variables_) {
    if (pair.second.isString()) {
      strVars[pair.first] = pair.second.getString();
    }
  }
  return strVars;
}
