#pragma once

#include "types.h"
#include <map>
#include <vector>
#include <string>

class Variables {
public:
    Variables();
    
    // Variable operations
    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name);
    bool hasVariable(const std::string& name) const;
    void clear();
    
    // Array operations
    void dimArray(const std::string& name, const std::vector<int>& dimensions);
    void setArrayElement(const std::string& name, const std::vector<int>& indices, const Value& value);
    Value getArrayElement(const std::string& name, const std::vector<int>& indices);
    
    // User-defined functions
    void defineFunction(const std::string& name, const std::string& param, const std::string& expr);
    bool hasFunction(const std::string& name) const;
    std::pair<std::string, std::string> getFunction(const std::string& name);
    
private:
    std::map<std::string, Value> variables_;
    
    struct ArrayInfo {
        std::vector<int> dimensions;
        std::map<std::vector<int>, Value> data;
    };
    std::map<std::string, ArrayInfo> arrays_;
    
    struct FunctionInfo {
        std::string parameter;
        std::string expression;
    };
    std::map<std::string, FunctionInfo> functions_;
    
    // Normalize variable name (Applesoft only uses first 2 chars for non-array variables)
    std::string normalizeName(const std::string& name) const;
};
