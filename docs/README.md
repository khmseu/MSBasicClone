# MSBasic Documentation

This directory contains all documentation for the MSBasic project.

## Documentation Files

### Architecture Documentation
- **[architecture.md](architecture.md)**: Comprehensive overview of the MSBasic system architecture, including component descriptions, data flow, and design decisions.

### Features Documentation
- **[features.md](features.md)**: Implementation status tracking document listing all implemented features, test coverage, and future enhancements.

### API Documentation
- **api/** (generated): Doxygen-generated API documentation. Build with `make docs` or `cmake --build build --target docs`.

## Building API Documentation

The project uses Doxygen to generate API documentation from source code comments.

### Requirements
- Doxygen 1.8.0 or later
- (Optional) LaTeX for PDF generation
- (Optional) Graphviz for diagrams

### Build Instructions

1. **Install Doxygen** (if not already installed):
   ```bash
   # Ubuntu/Debian
   sudo apt-get install doxygen graphviz
   
   # macOS
   brew install doxygen graphviz
   
   # Windows (with Chocolatey)
   choco install doxygen.install graphviz
   ```

2. **Build HTML Documentation**:
   ```bash
   cd /path/to/MSBasicClone
   cmake --build build --target docs
   ```
   
   Or directly with Doxygen:
   ```bash
   doxygen Doxyfile
   ```

3. **View Documentation**:
   ```bash
   # HTML output
   open docs/api/html/index.html    # macOS
   xdg-open docs/api/html/index.html  # Linux
   start docs/api/html/index.html   # Windows
   ```

4. **Build PDF Documentation** (optional):
   ```bash
   cd docs/api/latex
   make
   # Output: refman.pdf
   ```

### Documentation Structure

The Doxygen configuration (`Doxyfile` in the project root) generates:
- **HTML**: Browsable web documentation with search capability
- **LaTeX**: Source files for PDF generation
- **Cross-references**: Links between related classes, functions, and files
- **Call graphs**: Visual representation of function dependencies (requires Graphviz)
- **Class diagrams**: Inheritance and collaboration relationships

## Documentation Coverage

### Source Files with Doxygen Comments
- `src/types.h` - Core type definitions
- `src/tokenizer.h` - Lexical analysis
- `src/parser.h` - Syntax analysis
- `src/interpreter.h` - Runtime execution
- `src/variables.h` - Variable management
- `src/functions.h` - Built-in functions
- `src/statements.h` - Statement implementations
- `src/graphics.h` - Graphics subsystem
- `src/float40.h` - 40-bit floating-point
- Additional files documented inline

### Markdown Documentation
- `README.md` - Project overview and quick start
- `docs/architecture.md` - System architecture
- `docs/features.md` - Implementation status
- `Research/*.md` - Applesoft BASIC reference materials

## Documentation Guidelines

When contributing code, please follow these documentation standards:

### File Header
```cpp
/**
 * @file filename.h
 * @brief Brief description of the file's purpose
 * 
 * More detailed description of what this file contains,
 * its role in the system, and any important notes.
 */
```

### Class Documentation
```cpp
/**
 * @class ClassName
 * @brief Brief description of the class
 * 
 * Detailed description of the class purpose, responsibilities,
 * usage patterns, and any important design considerations.
 */
class ClassName {
  // ...
};
```

### Function/Method Documentation
```cpp
/**
 * @brief Brief description of what the function does
 * 
 * Detailed description of the function's behavior,
 * side effects, and usage notes.
 * 
 * @param paramName Description of parameter
 * @param otherParam Description of another parameter
 * @return Description of return value
 * @throws ExceptionType Description of when/why thrown
 */
ReturnType functionName(Type paramName, OtherType otherParam);
```

### Inline Comments
Use `//` for single-line explanatory comments and `/* */` for multi-line comments.
Focus on explaining *why* rather than *what* (the code shows what).

## Maintenance

### Updating Documentation
1. Update source code comments when changing implementations
2. Regenerate Doxygen docs: `cmake --build build --target docs`
3. Update architecture.md for design changes
4. Update features.md for new features or status changes

### Documentation Testing
Verify documentation builds without errors:
```bash
doxygen Doxyfile 2>&1 | grep -i "warning\|error"
```

Should produce no warnings or errors for clean documentation.

## Additional Resources

- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Doxygen Special Commands](https://www.doxygen.nl/manual/commands.html)
- [Markdown in Doxygen](https://www.doxygen.nl/manual/markdown.html)

---

*Last Updated: 2025-12-21*
