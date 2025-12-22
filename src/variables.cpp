/**
 * @file variables.cpp
 * @brief Implementation of variable, array, and function storage
 * 
 * This file implements the Variables class which manages all runtime storage
 * for BASIC programs. It enforces Applesoft BASIC variable naming conventions
 * (first 2 characters significant), handles type coercion (especially for
 * integer variables), and provides sparse array storage.
 * 
 * Key implementation details:
 * - Variable names normalized to uppercase, 2-char significance
 * - FN functions preserve FN prefix + 2 chars (4 chars total)
 * - Integer variables (%) clamped to 16-bit signed range (-32768 to 32767)
 * - Arrays auto-dimension to size 10 per dimension if not explicitly DIM'd
 * - Sparse array storage using std::map keyed by dimension indices
 */

#include "variables.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
// Applesoft BASIC variable name significance limits
constexpr size_t VARIABLE_NAME_LENGTH = 2;
constexpr size_t FN_FUNCTION_NAME_LENGTH = 4; // FN prefix + 2 chars
} // namespace

/**
 * @brief Construct a new Variables storage system
 */
Variables::Variables() {}

/**
 * @brief Normalize a variable name according to Applesoft conventions
 * 
 * Implements Applesoft BASIC's variable name significance rules:
 * - Convert to uppercase
 * - For FN functions: preserve "FN" prefix + 2 chars (e.g., FNxy)
 * - For other variables: use first 2 chars (unless string/integer suffix)
 * - Suffixes $ and % are preserved
 * 
 * Examples:
 *   "hello" -> "HE"
 *   "HELLO$" -> "HELLO$"
 *   "counter%" -> "CO%"
 *   "FNabc" -> "FNAB"
 * 
 * @param name Variable name to normalize
 * @return Normalized name
 */
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
/**
 * @brief Coerce a value to integer range
 * 
 * Integer variables in Applesoft BASIC are 16-bit signed, ranging from
 * -32768 to 32767. This function rounds the numeric value and clamps it
 * to that range.
 * 
 * @param value Value to coerce
 * @return Clamped integer value
 */
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

/**
 * @brief Set a variable value
 * 
 * Stores a value in a variable, applying type coercion for integer variables.
 * Variable names are normalized according to Applesoft rules (2-char significance).
 * 
 * Type handling:
 * - Integer variables (name ends with %): value is clamped to 16-bit signed range
 * - String variables (name ends with $): value stored as-is
 * - Numeric variables: value stored as-is (Float40 precision)
 * 
 * Examples:
 *   setVariable("COUNTER%", 100)   → stores 100 as integer
 *   setVariable("NAME$", "HELLO")  → stores "HELLO" as string
 *   setVariable("X", 3.14159)      → stores 3.14159 as number
 * 
 * @param name Variable name (may include $ or % suffix)
 * @param value Value to store (type coercion applied for %)
 */
void Variables::setVariable(const std::string &name, const Value &value) {
  if (!name.empty() && name.back() == '%') {
    variables_[normalizeName(name)] = coerceInteger(value);
    return;
  }
  variables_[normalizeName(name)] = value;
}

/**
 * @brief Remove a variable from storage
 * 
 * Deletes a variable from memory. If the variable doesn't exist,
 * this operation silently succeeds.
 * 
 * @param name Variable name to remove
 */
void Variables::unsetVariable(const std::string &name) {
  variables_.erase(normalizeName(name));
}

/**
 * @brief Get a variable value
 * 
 * Retrieves the value of a variable. Returns default values for uninitialized
 * variables following Applesoft BASIC conventions:
 * - String variables ($): empty string ""
 * - Numeric variables: 0.0
 * 
 * Examples:
 *   getVariable("X")      → returns 0.0 if X not set
 *   getVariable("NAME$")  → returns "" if NAME$ not set
 *   getVariable("COUNT%") → returns 0 if COUNT% not set
 * 
 * @param name Variable name to retrieve
 * @return Variable value, or default if uninitialized
 */
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

/**
 * @brief Check if a variable exists
 * 
 * Tests whether a variable has been explicitly set. Returns false for
 * variables that would return default values (0 or "").
 * 
 * @param name Variable name to check
 * @return true if variable has been set, false otherwise
 */
bool Variables::hasVariable(const std::string &name) const {
  return variables_.find(normalizeName(name)) != variables_.end();
}

/**
 * @brief Clear all variables and arrays
 * 
 * Removes all variables and arrays from memory. User-defined functions
 * (DEF FN) are preserved, matching Applesoft BASIC behavior where
 * functions persist across CLEAR commands.
 * 
 * This is called by the CLR and NEW commands.
 */
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

/**
 * @brief Check if a user-defined function exists
 * 
 * Tests whether a function has been defined with DEF FN.
 * 
 * @param name Function name (e.g., "FNXY")
 * @return true if function is defined, false otherwise
 */
bool Variables::hasFunction(const std::string &name) const {
  return functions_.find(normalizeName(name)) != functions_.end();
}

/**
 * @brief Get user-defined function definition
 * 
 * Retrieves the function definition for evaluation. Function calls
 * substitute the parameter value and evaluate the function body expression.
 * 
 * @param name Function name (e.g., "FNXY")
 * @return Function information (parameter name and body expression)
 * @throws std::runtime_error if function is not defined
 */
const Variables::FunctionInfo &Variables::getFunction(const std::string &name) {
  std::string normalized = normalizeName(name);
  auto it = functions_.find(normalized);
  if (it == functions_.end()) {
    throw std::runtime_error("UNDEFINED FUNCTION ERROR");
  }
  return it->second;
}

/**
 * @brief Check if an array has been dimensioned
 * 
 * Tests whether an array has been explicitly dimensioned with DIM
 * or auto-dimensioned by first use.
 * 
 * @param name Array name
 * @return true if array exists, false otherwise
 */
bool Variables::hasArray(const std::string &name) const {
  return arrays_.find(normalizeName(name)) != arrays_.end();
}

/**
 * @brief Get array dimensions
 * 
 * Returns the dimension sizes for a dimensioned array.
 * 
 * Example:
 *   DIM A(5,10) creates array with dimensions [5, 10]
 * 
 * @param name Array name
 * @return Vector of dimension sizes
 * @throws std::runtime_error if array not defined
 */
const std::vector<int> &Variables::getArrayDimensions(const std::string &name) const {
  std::string normalized = normalizeName(name);
  auto it = arrays_.find(normalized);
  if (it == arrays_.end()) {
    throw std::runtime_error("UNDEFINED ARRAY ERROR");
  }
  return it->second.dimensions;
}

/**
 * @brief Get array data storage
 * 
 * Returns the sparse array data map. Array elements are stored as
 * map entries keyed by dimension indices. Uninitialized elements
 * are not stored (sparse storage).
 * 
 * @param name Array name
 * @return Map of dimension indices to values
 * @throws std::runtime_error if array not defined
 */
const std::map<std::vector<int>, Value> &
Variables::getArrayData(const std::string &name) const {
  std::string normalized = normalizeName(name);
  auto it = arrays_.find(normalized);
  if (it == arrays_.end()) {
    throw std::runtime_error("UNDEFINED ARRAY ERROR");
  }
  return it->second.data;
}

/**
 * @brief Set array data (bulk restore)
 * 
 * Sets the complete array dimensions and data. Used by RECALL command
 * to restore arrays from tape storage.
 * 
 * @param name Array name
 * @param dimensions Array dimension sizes
 * @param data Complete array data map
 */
void Variables::setArrayData(const std::string &name,
                             const std::vector<int> &dimensions,
                             const std::map<std::vector<int>, Value> &data) {
  std::string normalized = normalizeName(name);
  ArrayInfo &arr = arrays_[normalized];
  arr.dimensions = dimensions;
  arr.data = data;
}

/**
 * @brief Get all numeric variables
 * 
 * Returns a map of all numeric variables (non-string) for iteration
 * or inspection. Used for debugging and state export.
 * 
 * @return Map of normalized variable names to numeric values
 */
std::map<std::string, double> Variables::getAllNumericVariables() const {
  std::map<std::string, double> numVars;
  for (const auto &pair : variables_) {
    if (!pair.second.isString()) {
      numVars[pair.first] = pair.second.getNumber();
    }
  }
  return numVars;
}

/**
 * @brief Get all string variables
 * 
 * Returns a map of all string variables for iteration or inspection.
 * Used for debugging and state export.
 * 
 * @return Map of normalized variable names to string values
 */
std::map<std::string, std::string> Variables::getAllStringVariables() const {
  std::map<std::string, std::string> strVars;
  for (const auto &pair : variables_) {
    if (pair.second.isString()) {
      strVars[pair.first] = pair.second.getString();
    }
  }
  return strVars;
}
