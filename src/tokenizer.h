/**
 * @file tokenizer.h
 * @brief Lexical analyzer (tokenizer) for BASIC source code
 * 
 * The tokenizer performs lexical analysis, converting raw BASIC source text
 * into a stream of typed tokens. It recognizes keywords, identifiers, literals,
 * operators, and delimiters, preparing the input for the parser.
 * 
 * Key responsibilities:
 * - Keyword recognition (PRINT, IF, FOR, etc.)
 * - Number parsing (integers and floating-point)
 * - String literal parsing (with escape sequences)
 * - Identifier recognition (variables, FN names, etc.)
 * - Operator and delimiter recognition
 * - Position tracking for error reporting
 */

#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class Tokenizer
 * @brief Converts BASIC source code into a stream of tokens
 * 
 * The Tokenizer class performs lexical analysis on BASIC source code,
 * breaking it down into tokens that the parser can process. It handles
 * all Applesoft BASIC syntax including keywords, operators, literals,
 * and identifiers.
 * 
 * Usage:
 * @code
 * Tokenizer tokenizer;
 * std::vector<Token> tokens = tokenizer.tokenize("PRINT \"HELLO\"");
 * @endcode
 * 
 * The tokenizer maintains position information (line and column) for
 * error reporting and supports both immediate commands and program lines.
 */
class Tokenizer {
public:
    /**
     * @brief Construct a new Tokenizer
     */
    Tokenizer();
    
    /**
     * @brief Tokenize a line of BASIC code
     * 
     * Converts a string of BASIC source code into a vector of tokens.
     * The tokenizer handles all Applesoft BASIC syntax including
     * multi-statement lines (separated by colons).
     * 
     * @param line The source code line to tokenize
     * @return std::vector<Token> The sequence of tokens
     * @throws std::runtime_error On lexical errors (unterminated strings, etc.)
     */
    std::vector<Token> tokenize(const std::string& line);
    
    /**
     * @brief Check if a string is a BASIC keyword
     * 
     * @param word The string to check
     * @return true if the word is a recognized BASIC keyword
     * @return false otherwise
     */
    bool isKeyword(const std::string& word) const;
    
    /**
     * @brief Get the token type for a keyword
     * 
     * @param word The keyword string
     * @return TokenType The corresponding token type
     * @throws std::runtime_error if word is not a keyword
     */
    TokenType getKeywordType(const std::string& word) const;
    
private:
    /** @brief Skip whitespace characters in the input */
    void skipWhitespace();
    
    /** @brief Read the next token from the input stream */
    Token nextToken();
    
    /** @brief Read a numeric literal token */
    Token readNumber();
    
    /** @brief Read a string literal token */
    Token readString();
    
    /** @brief Read an identifier or keyword token */
    Token readIdentifier();
    
    /** @brief Read an operator token */
    Token readOperator();
    
    /** @brief The input string being tokenized */
    std::string input_;
    
    /** @brief Current position in the input string */
    size_t pos_;
    
    /** @brief Current line number (for error reporting) */
    int line_;
    
    /** @brief Current column number (for error reporting) */
    int column_;
    
    /**
     * @brief Peek at the current character without consuming it
     * @return The current character, or '\0' if at end
     */
    char peek() const;
    
    /**
     * @brief Consume and return the current character
     * @return The current character before advancing
     */
    char advance();
    
    /**
     * @brief Check if we're at the end of input
     * @return true if at end of input, false otherwise
     */
    bool isAtEnd() const;
};
