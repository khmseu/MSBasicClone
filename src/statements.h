/**
 * @file statements.h
 * @brief Statement implementations for Applesoft BASIC
 * 
 * This header declares statement-related types and implementations.
 * Most statement logic is implemented directly in the Parser and Interpreter
 * classes as methods, with this file serving as a placeholder for future
 * statement class hierarchies if needed.
 * 
 * Statements are represented as AST nodes created by the Parser and executed
 * by the Interpreter. The actual implementations are in parser.cpp and
 * interpreter.cpp rather than separate statement classes.
 * 
 * For statement documentation, see:
 * - Parser::parseStatement() for statement syntax parsing
 * - Interpreter methods for statement execution
 */

#pragma once

// Statement implementations
#include "types.h"
#include "interpreter.h"
#include "parser.h"
#include <memory>

// Various statement classes will be defined here
// For now, we'll implement them in parser.cpp
