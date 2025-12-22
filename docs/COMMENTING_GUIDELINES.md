# Code Commenting Guidelines

This document describes the commenting standards for the MSBasic interpreter codebase, established to improve code maintainability, understanding, and API documentation generation.

## Overview

All implementation files should have detailed structured comments that explain:
- **What** the code does (brief summary)
- **How** it works (algorithm details)
- **Why** design decisions were made (rationale)
- **When** to use or avoid certain features (usage guidance)
- **Edge cases** and error conditions

## Documentation Style

### Doxygen Format

All comments use Doxygen-style formatting for API documentation generation:

```cpp
/**
 * @brief Brief one-line description
 * 
 * Detailed multi-paragraph explanation with algorithm details,
 * implementation notes, and usage guidance.
 * 
 * Additional sections as needed:
 * - Behavior details
 * - Implementation notes
 * - Platform-specific considerations
 * 
 * @param paramName Description of parameter with constraints
 * @return Description of return value
 * @throws Exception description for error conditions
 */
```

### Required Elements

1. **File Headers**: Every implementation file should have a comprehensive header:
   ```cpp
   /**
    * @file filename.cpp
    * @brief One-line purpose
    * 
    * Detailed description covering:
    * - Key responsibilities
    * - Architecture/design patterns
    * - Platform considerations
    * - Important algorithms
    */
   ```

2. **Function Documentation**: Every non-trivial function should have:
   - Brief description (@brief)
   - Detailed explanation of algorithm/behavior
   - Parameter descriptions (@param)
   - Return value documentation (@return)
   - Exception documentation (@throws)
   - Usage examples where helpful

3. **Implementation Notes**: Use inline comments for:
   - Complex algorithm steps
   - Non-obvious logic
   - Platform-specific code
   - Workarounds or special cases

## Content Guidelines

### What to Document

#### High Priority
- **Complex algorithms**: Step-by-step breakdowns
- **Memory operations**: Address mappings, special locations
- **Control flow**: State machines, jump logic, stack operations
- **Transformations**: Mathematical formulas, coordinate systems
- **Platform-specific code**: OS-dependent implementations
- **Error handling**: Conditions, recovery strategies
- **Compatibility notes**: Applesoft BASIC semantics

#### Medium Priority
- **Data structures**: Purpose and invariants
- **Helper functions**: Purpose and usage context
- **Constants**: Meaning and origin
- **Type conversions**: Coercion rules

#### Lower Priority
- **Trivial getters/setters**: Brief @brief is sufficient
- **Standard patterns**: Well-known idioms need less explanation
- **Self-documenting code**: Clear variable names reduce need for comments

### Usage Examples

Include BASIC syntax examples for user-facing features:

```cpp
/**
 * @brief Draw horizontal line (HLIN implementation)
 * 
 * Draws a horizontal line from (x1,y) to (x2,y).
 * 
 * Usage in BASIC:
 *   HLIN 0,39 AT 20  (draw line across screen)
 *   HLIN 100,200 AT 96  (HGR mode horizontal line)
 * 
 * @param x1 Starting X coordinate
 * @param x2 Ending X coordinate
 * @param y Y coordinate (constant for horizontal line)
 */
```

### Mathematical Formulas

Document transformation math explicitly:

```cpp
/**
 * Transformation math:
 *   rotated_x = x * cos(angle) - y * sin(angle)
 *   rotated_y = x * sin(angle) + y * cos(angle)
 *   final_x = origin_x + rotated_x * scale
 *   final_y = origin_y + rotated_y * scale
 */
```

### Algorithm Breakdowns

Use numbered steps for complex processes:

```cpp
/**
 * Execution process:
 * 1. Initialize execution state (running_ flag, immediate_ mode)
 * 2. Build DATA cache by scanning all program lines
 *    - This ensures READ can access DATA regardless of program flow
 *    - dataOffsets_ maps line numbers to positions in dataValues_
 * 3. Set program counter to starting line
 * 4. Execute statements sequentially until:
 *    - END or STOP statement sets running_ = false
 *    - GOTO/GOSUB sets jumped_ flag
 *    - Error occurs
 *    - Program counter reaches end
 */
```

## Examples by Category

### Memory Operations

```cpp
/**
 * @brief Write a byte value to memory (POKE implementation)
 * 
 * Implements the POKE statement which writes a byte to a memory address.
 * This function handles the complex address mapping of the Apple II including
 * special system addresses, I/O locations, and memory-mapped hardware.
 * 
 * Address Handling:
 * - Negative addresses are converted to 16-bit unsigned (Apple II convention)
 *   Example: POKE -16368,0 becomes POKE 49168,0 (clear keyboard strobe)
 * - Values are masked to 8 bits (0-255) to simulate byte storage
 * 
 * Special Addresses (bypass range checking):
 * - Zero page system variables (0x00-0xFF)
 * - Text window control (0x20-0x25)
 * - Memory pointers: LOMEM (0x69-0x6A), HIMEM (0x73-0x74)
 * [... more details ...]
 * 
 * @param addr Memory address to write to (may be negative)
 * @param val Value to write (will be masked to 0-255)
 * @throws std::runtime_error If address is outside valid range
 */
```

### Parser/Language Features

```cpp
/**
 * @brief Parse logical OR expressions (lowest precedence)
 * 
 * Handles OR operator which has lowest precedence. Parses left-associative
 * chains of OR operations:
 *   A OR B OR C = (A OR B) OR C
 * 
 * In Applesoft BASIC, OR performs bitwise OR on integer values:
 *   0 OR 0 = 0 (false OR false = false)
 *   0 OR 1 = 1 (false OR true = true)
 *   Any non-zero value is considered true
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing OR chain or single AND expression
 */
```

### Graphics Operations

```cpp
/**
 * @brief Draw shape with XOR mode (XDRAW implementation)
 * 
 * Similar to draw() but uses XOR (exclusive or) mode for drawing. In XOR mode:
 * - Drawing on background sets pixels
 * - Drawing on existing pixels clears them
 * - Drawing the same shape twice returns to original state
 * 
 * XOR drawing is useful for:
 * - Animation (draw/erase/move/redraw without clearing screen)
 * - Cursors and temporary graphics
 * - Creating "invert" effects
 * 
 * XOR Logic:
 * - If pixel is off (0), set to current color
 * - If pixel is on (non-zero), set to off (0)
 * - This makes drawing reversible: XDRAW twice = no change
 * 
 * Usage in BASIC:
 *   XDRAW 1 AT 100,100  (draw shape 1)
 *   XDRAW 1 AT 100,100  (erase shape 1 - back to original)
 * 
 * @param shapeNum Shape number to draw in XOR mode
 * @param x X coordinate of origin (< 0 uses last position)
 * @param y Y coordinate of origin (< 0 uses last position)
 */
```

## Files Requiring Documentation

### High Priority (Large Implementation Files)
- `src/interpreter.cpp` - Runtime execution engine
- `src/parser.cpp` - Syntax analyzer and AST builder
- `src/functions.cpp` - Built-in functions and memory operations
- `src/graphics.cpp` - Graphics operations and transformations

### Medium Priority
- `src/statements.cpp` - Statement implementations
- `src/tokenizer.cpp` - Lexical analyzer
- `src/variables.cpp` - Variable storage management

### Already Well-Documented
- `src/float40.cpp` - 40-bit float emulation
- `src/filesystem.cpp` - Cross-platform file operations
- Header files (.h) - API documentation

## Validation

### Documentation Quality Checklist

- [ ] File header explains purpose and architecture
- [ ] Complex functions have detailed explanations
- [ ] Algorithms are broken down step-by-step
- [ ] Usage examples included for user-facing features
- [ ] Edge cases and error conditions documented
- [ ] Platform-specific behavior noted
- [ ] Mathematical formulas included where applicable
- [ ] Doxygen tags used correctly (@brief, @param, @return, @throws)
- [ ] Comments are accurate and up-to-date with code

### Build Validation

After adding documentation:
```bash
# Verify code still compiles
cmake -S . -B build
cmake --build build

# Verify tests still pass
ctest --test-dir build --output-on-failure

# Generate API documentation (if Doxygen available)
doxygen Doxyfile
```

## Anti-Patterns to Avoid

### Don't Document the Obvious
```cpp
// Bad: Obvious from code
int x = 5; // Set x to 5

// Good: Explains why
int maxRetries = 5; // Applesoft BASIC allows 5 input retries on type mismatch
```

### Don't Repeat Code in Words
```cpp
// Bad: Just repeats the code
/**
 * @brief Adds two numbers
 * This function takes two numbers and adds them together.
 */
int add(int a, int b) { return a + b; }

// Good: Adds context
/**
 * @brief Add with Float40 precision
 * 
 * Performs addition with Applesoft-compatible 40-bit floating point
 * precision, matching Apple II BASIC behavior for calculations.
 */
Float40 add(Float40 a, Float40 b);
```

### Don't Leave Outdated Comments
```cpp
// Bad: Comment doesn't match code
// Returns true if file exists
bool validatePath(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

// Good: Accurate description
/**
 * @brief Validate path is an existing regular file
 * @return true if path exists and is a regular file, false otherwise
 */
bool validatePath(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}
```

## Summary

Good documentation:
- Explains complex algorithms with step-by-step breakdowns
- Includes usage examples in BASIC syntax
- Documents edge cases and error conditions
- Uses Doxygen format for API generation
- Provides context and rationale, not just code repetition
- Stays accurate and up-to-date with implementation

This approach makes the codebase maintainable, debuggable, and accessible to new contributors while preserving knowledge about Applesoft BASIC compatibility requirements.
