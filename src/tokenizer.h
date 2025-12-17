#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <memory>

class Tokenizer {
public:
    Tokenizer();
    
    // Tokenize a line of BASIC code
    std::vector<Token> tokenize(const std::string& line);
    
    // Check if a string is a keyword
    bool isKeyword(const std::string& word) const;
    
    // Get keyword token type
    TokenType getKeywordType(const std::string& word) const;
    
private:
    void skipWhitespace();
    Token nextToken();
    Token readNumber();
    Token readString();
    Token readIdentifier();
    Token readOperator();
    
    std::string input_;
    size_t pos_;
    int line_;
    int column_;
    
    char peek() const;
    char advance();
    bool isAtEnd() const;
};
