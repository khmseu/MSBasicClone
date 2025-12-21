/**
 * @file variables.h
 * @brief Variable, array, and user-defined function storage and management
 * 
 * The Variables class manages all runtime storage for BASIC programs including:
 * - Simple variables (numeric, string, integer)
 * - Multi-dimensional arrays with auto-dimensioning
 * - User-defined functions (DEF FN)
 * 
 * Key features:
 * - Applesoft-compatible variable name normalization (first 2 chars significant)
 * - Sparse array storage for memory efficiency
 * - Type-safe value storage with automatic coercion
 * - Function parameter substitution
 */

#pragma once

#include "types.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class Expression;

/**
 * @class Variables
 * @brief Storage and management for program variables, arrays, and functions
 * 
 * The Variables class implements the variable storage system for MSBasic,
 * following Applesoft BASIC conventions for variable names and types.
 * 
 * Variable naming rules:
 * - Only first 2 characters are significant (case-insensitive)
 * - Exception: FN names preserve FN prefix + 2 chars (e.g., FNxy)
 * - '$' suffix denotes string variables
 * - '%' suffix denotes integer variables (16-bit clamped)
 * 
 * Arrays:
 * - Multi-dimensional with arbitrary dimension count
 * - Auto-dimension to size 10 per dimension if undeclared
 * - Sparse storage using std::map for memory efficiency
 * - Bounds checking with "BAD SUBSCRIPT" error
 * 
 * User-defined functions:
 * - Stored with parameter name and expression AST
 * - Evaluated by substituting parameter value
 * - Follow FN naming conventions
 */
class Variables {
public:
  /**
   * @brief Construct a new Variables storage system
   */
  Variables();

  // Variable operations
  
  /**
   * @brief Set a variable's value
   * 
   * Creates the variable if it doesn't exist, or updates its value.
   * Variable name is normalized according to Applesoft conventions.
   * 
   * @param name Variable name (may include $, %, or FN prefix)
   * @param value The value to store
   * @throws std::runtime_error on type mismatch (e.g., string to numeric)
   */
  void setVariable(const std::string &name, const Value &value);
  
  /**
   * @brief Remove a variable
   * 
   * @param name Variable name to remove
   */
  void unsetVariable(const std::string &name);
  
  /**
   * @brief Get a variable's value
   * 
   * @param name Variable name to lookup
   * @return Value The variable's current value
   * @throws std::runtime_error if variable doesn't exist
   */
  Value getVariable(const std::string &name);
  
  /**
   * @brief Check if a variable exists
   * 
   * @param name Variable name to check
   * @return true if variable is defined, false otherwise
   */
  bool hasVariable(const std::string &name) const;
  
  /**
   * @brief Clear all variables, arrays, and functions
   * 
   * Resets the Variables object to initial state. Called by CLR command.
   */
  void clear();

  // Array operations
  
  /**
   * @brief Dimension an array with specified sizes
   * 
   * Creates a new array with the given dimensions. Arrays are stored
   * sparsely - elements are created on demand.
   * 
   * @param name Array name (normalized)
   * @param dimensions Vector of dimension sizes (each must be > 0)
   * @throws std::runtime_error if array already dimensioned (REDIM error)
   */
  void dimArray(const std::string &name, const std::vector<int> &dimensions);
  
  /**
   * @brief Set an array element's value
   * 
   * Auto-dimensions the array to size 10 per dimension if not yet declared.
   * Creates the element if it doesn't exist.
   * 
   * @param name Array name
   * @param indices Subscript values for each dimension
   * @param value The value to store
   * @throws std::runtime_error on bad subscript (out of bounds)
   */
  void setArrayElement(const std::string &name, const std::vector<int> &indices,
                       const Value &value);
  
  /**
   * @brief Get an array element's value
   * 
   * Auto-dimensions the array to size 10 per dimension if not yet declared.
   * Returns Value(0.0) for uninitialized numeric elements.
   * 
   * @param name Array name
   * @param indices Subscript values for each dimension
   * @return Value The element's value
   * @throws std::runtime_error on bad subscript (out of bounds)
   */
  Value getArrayElement(const std::string &name,
                        const std::vector<int> &indices);

  // User-defined functions
  
  /**
   * @brief Define a user function (DEF FN)
   * 
   * Stores a user-defined function with its parameter and body expression.
   * Function calls will substitute the parameter value and evaluate the body.
   * 
   * @param name Function name (e.g., "FNxy")
   * @param param Parameter name (e.g., "x")
   * @param expr Expression AST for the function body
   */
  void defineFunction(const std::string &name, const std::string &param,
                      const std::shared_ptr<Expression> &expr);
  
  /**
   * @brief Check if a function is defined
   * 
   * @param name Function name to check
   * @return true if function is defined, false otherwise
   */
  bool hasFunction(const std::string &name) const;
  
  /**
   * @struct FunctionInfo
   * @brief Storage for user-defined function details
   */
  struct FunctionInfo {
    /** @brief Parameter variable name */
    std::string parameter;
    /** @brief Function body expression AST */
    std::shared_ptr<Expression> body;
  };

  /**
   * @brief Get function definition
   * 
   * @param name Function name
   * @return const FunctionInfo& Function details
   * @throws std::runtime_error if function not defined
   */
  const FunctionInfo &getFunction(const std::string &name);

  // Array persistence helpers
  
  /**
   * @brief Check if an array exists
   * 
   * @param name Array name to check
   * @return true if array is dimensioned, false otherwise
   */
  bool hasArray(const std::string &name) const;
  
  /**
   * @brief Get array dimensions
   * 
   * @param name Array name
   * @return const std::vector<int>& Vector of dimension sizes
   * @throws std::runtime_error if array doesn't exist
   */
  const std::vector<int> &getArrayDimensions(const std::string &name) const;
  
  /**
   * @brief Get array data (for persistence)
   * 
   * Returns the sparse map of array elements for saving to disk.
   * 
   * @param name Array name
   * @return const std::map<std::vector<int>, Value>& Array element map
   * @throws std::runtime_error if array doesn't exist
   */
  const std::map<std::vector<int>, Value> &getArrayData(const std::string &name) const;
  
  /**
   * @brief Set array data (for restoration)
   * 
   * Replaces array contents with loaded data. Used by RECALL command.
   * 
   * @param name Array name
   * @param dimensions Dimension sizes
   * @param data Map of element values
   */
  void setArrayData(const std::string &name, const std::vector<int> &dimensions,
                   const std::map<std::vector<int>, Value> &data);

  // Variable persistence helpers (for ProDOS STORE/RESTORE)
  
  /**
   * @brief Get all numeric variables
   * 
   * Returns a map of all numeric (non-string, non-integer) variables
   * for saving to disk via ProDOS STORE command.
   * 
   * @return std::map<std::string, double> Map of variable names to values
   */
  std::map<std::string, double> getAllNumericVariables() const;
  
  /**
   * @brief Get all string variables
   * 
   * Returns a map of all string variables (names ending in $)
   * for saving to disk via ProDOS STORE command.
   * 
   * @return std::map<std::string, std::string> Map of variable names to values
   */
  std::map<std::string, std::string> getAllStringVariables() const;

private:
  /** @brief Map of simple variables (normalized name -> value) */
  std::map<std::string, Value> variables_;

  /**
   * @struct ArrayInfo
   * @brief Storage structure for array metadata and data
   */
  struct ArrayInfo {
    /** @brief Size of each dimension */
    std::vector<int> dimensions;
    /** @brief Sparse map of element indices to values */
    std::map<std::vector<int>, Value> data;
  };
  
  /** @brief Map of arrays (normalized name -> array info) */
  std::map<std::string, ArrayInfo> arrays_;

  /** @brief Map of user-defined functions (normalized name -> function info) */
  std::map<std::string, FunctionInfo> functions_;

  /**
   * @brief Normalize variable name according to Applesoft conventions
   * 
   * Normalization rules:
   * - Convert to uppercase
   * - Extract first 2 characters (excluding type suffixes)
   * - Preserve $ and % suffixes
   * - Special handling for FN names (preserve FN + 2 chars)
   * 
   * Examples:
   * - "Hello" -> "HE"
   * - "Name$" -> "NA$"
   * - "Count%" -> "CO%"
   * - "FNabc" -> "FNAB"
   * 
   * @param name Original variable name
   * @return std::string Normalized name
   */
  std::string normalizeName(const std::string &name) const;
};
