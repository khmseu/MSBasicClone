#pragma once

#include "types.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

class Expression;

class Variables {
public:
  Variables();

  // Variable operations
  void setVariable(const std::string &name, const Value &value);
  void unsetVariable(const std::string &name);
  Value getVariable(const std::string &name);
  bool hasVariable(const std::string &name) const;
  void clear();

  // Array operations
  void dimArray(const std::string &name, const std::vector<int> &dimensions);
  void setArrayElement(const std::string &name, const std::vector<int> &indices,
                       const Value &value);
  Value getArrayElement(const std::string &name,
                        const std::vector<int> &indices);

  // User-defined functions
  void defineFunction(const std::string &name, const std::string &param,
                      const std::shared_ptr<Expression> &expr);
  bool hasFunction(const std::string &name) const;
  struct FunctionInfo {
    std::string parameter;
    std::shared_ptr<Expression> body;
  };

  const FunctionInfo &getFunction(const std::string &name);

  // Array persistence helpers
  bool hasArray(const std::string &name) const;
  const std::vector<int> &getArrayDimensions(const std::string &name) const;
  const std::map<std::vector<int>, Value> &getArrayData(const std::string &name) const;
  void setArrayData(const std::string &name, const std::vector<int> &dimensions,
                   const std::map<std::vector<int>, Value> &data);

private:
  std::map<std::string, Value> variables_;

  struct ArrayInfo {
    std::vector<int> dimensions;
    std::map<std::vector<int>, Value> data;
  };
  std::map<std::string, ArrayInfo> arrays_;

  std::map<std::string, FunctionInfo> functions_;

  // Normalize variable name (Applesoft only uses first 2 chars for non-array
  // variables)
  std::string normalizeName(const std::string &name) const;
};
