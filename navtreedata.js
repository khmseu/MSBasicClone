/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "MSBasic - Applesoft BASIC Interpreter", "index.html", [
    [ "Applesoft BASIC Clone", "index.html", "index" ],
    [ "MSBasic Architecture Documentation", "md_docs_2architecture.html", [
      [ "Overview", "md_docs_2architecture.html#autotoc_md1", [
        [ "Design Goals", "md_docs_2architecture.html#autotoc_md2", null ]
      ] ],
      [ "System Architecture", "md_docs_2architecture.html#autotoc_md3", [
        [ "High-Level Components", "md_docs_2architecture.html#autotoc_md4", null ]
      ] ],
      [ "Component Details", "md_docs_2architecture.html#autotoc_md5", [
        [ "1. Tokenizer", "md_docs_2architecture.html#autotoc_md6", null ],
        [ "2. Parser", "md_docs_2architecture.html#autotoc_md7", null ],
        [ "3. Interpreter", "md_docs_2architecture.html#autotoc_md8", null ],
        [ "4. Variables", "md_docs_2architecture.html#autotoc_md9", null ],
        [ "5. Functions", "md_docs_2architecture.html#autotoc_md10", null ],
        [ "6. Statements", "md_docs_2architecture.html#autotoc_md11", null ],
        [ "7. Graphics Subsystem", "md_docs_2architecture.html#autotoc_md12", null ],
        [ "8. Float40", "md_docs_2architecture.html#autotoc_md13", null ],
        [ "9. Interactive Mode", "md_docs_2architecture.html#autotoc_md14", null ],
        [ "10. Filesystem", "md_docs_2architecture.html#autotoc_md15", null ],
        [ "11. Tape Manager", "md_docs_2architecture.html#autotoc_md16", null ]
      ] ],
      [ "Data Flow", "md_docs_2architecture.html#autotoc_md17", [
        [ "Program Execution Flow", "md_docs_2architecture.html#autotoc_md18", null ],
        [ "Expression Evaluation Flow", "md_docs_2architecture.html#autotoc_md19", null ],
        [ "Graphics Rendering Flow", "md_docs_2architecture.html#autotoc_md20", null ]
      ] ],
      [ "Memory Model", "md_docs_2architecture.html#autotoc_md21", [
        [ "Variable Storage", "md_docs_2architecture.html#autotoc_md22", null ],
        [ "Program Storage", "md_docs_2architecture.html#autotoc_md23", null ],
        [ "Simulated Memory", "md_docs_2architecture.html#autotoc_md24", null ],
        [ "Data Segment", "md_docs_2architecture.html#autotoc_md25", null ]
      ] ],
      [ "Build System", "md_docs_2architecture.html#autotoc_md26", [
        [ "CMake Configuration", "md_docs_2architecture.html#autotoc_md27", null ],
        [ "Font Integration", "md_docs_2architecture.html#autotoc_md28", null ]
      ] ],
      [ "Testing Strategy", "md_docs_2architecture.html#autotoc_md29", [
        [ "Test Suite", "md_docs_2architecture.html#autotoc_md30", null ],
        [ "Manual Testing", "md_docs_2architecture.html#autotoc_md31", null ]
      ] ],
      [ "Cross-Platform Considerations", "md_docs_2architecture.html#autotoc_md32", [
        [ "Platform-Specific Code", "md_docs_2architecture.html#autotoc_md33", null ],
        [ "Portability Techniques", "md_docs_2architecture.html#autotoc_md34", null ]
      ] ],
      [ "Error Handling", "md_docs_2architecture.html#autotoc_md35", [
        [ "Error Types", "md_docs_2architecture.html#autotoc_md36", null ],
        [ "Error Handling Flow", "md_docs_2architecture.html#autotoc_md37", null ],
        [ "Error Recovery", "md_docs_2architecture.html#autotoc_md38", null ]
      ] ],
      [ "Performance Considerations", "md_docs_2architecture.html#autotoc_md39", [
        [ "Optimizations", "md_docs_2architecture.html#autotoc_md40", null ],
        [ "Bottlenecks", "md_docs_2architecture.html#autotoc_md41", null ],
        [ "Scalability", "md_docs_2architecture.html#autotoc_md42", null ]
      ] ],
      [ "Future Enhancements", "md_docs_2architecture.html#autotoc_md43", [
        [ "Potential Improvements", "md_docs_2architecture.html#autotoc_md44", null ],
        [ "Backward Compatibility", "md_docs_2architecture.html#autotoc_md45", null ]
      ] ],
      [ "References", "md_docs_2architecture.html#autotoc_md46", null ],
      [ "Glossary", "md_docs_2architecture.html#autotoc_md47", null ]
    ] ],
    [ "Code Commenting Guidelines", "md_docs_2COMMENTING__GUIDELINES.html", [
      [ "Overview", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md50", null ],
      [ "Documentation Style", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md51", [
        [ "Doxygen Format", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md52", null ],
        [ "Required Elements", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md53", null ]
      ] ],
      [ "Content Guidelines", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md54", [
        [ "What to Document", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md55", [
          [ "High Priority", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md56", null ],
          [ "Medium Priority", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md57", null ],
          [ "Lower Priority", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md58", null ]
        ] ],
        [ "Usage Examples", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md59", null ],
        [ "Mathematical Formulas", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md60", null ],
        [ "Algorithm Breakdowns", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md61", null ]
      ] ],
      [ "Examples by Category", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md62", [
        [ "Memory Operations", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md63", null ],
        [ "Parser/Language Features", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md64", null ],
        [ "Graphics Operations", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md65", null ]
      ] ],
      [ "Files Requiring Documentation", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md66", [
        [ "High Priority (Large Implementation Files)", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md67", null ],
        [ "Medium Priority", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md68", null ],
        [ "Already Well-Documented", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md69", null ]
      ] ],
      [ "Validation", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md70", [
        [ "Documentation Quality Checklist", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md71", null ],
        [ "Build Validation", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md72", null ]
      ] ],
      [ "Anti-Patterns to Avoid", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md73", [
        [ "Don't Document the Obvious", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md74", null ],
        [ "Don't Repeat Code in Words", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md75", null ],
        [ "Don't Leave Outdated Comments", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md76", null ]
      ] ],
      [ "Summary", "md_docs_2COMMENTING__GUIDELINES.html#autotoc_md77", null ]
    ] ],
    [ "MSBasic Documentation", "md_docs_2docs__overview.html", [
      [ "Documentation Files", "md_docs_2docs__overview.html#autotoc_md79", [
        [ "Architecture Documentation", "md_docs_2docs__overview.html#autotoc_md80", null ],
        [ "Features Documentation", "md_docs_2docs__overview.html#autotoc_md81", null ],
        [ "API Documentation", "md_docs_2docs__overview.html#autotoc_md82", null ]
      ] ],
      [ "Building API Documentation", "md_docs_2docs__overview.html#autotoc_md83", [
        [ "Requirements", "md_docs_2docs__overview.html#autotoc_md84", null ],
        [ "Build Instructions", "md_docs_2docs__overview.html#autotoc_md85", null ],
        [ "Documentation Structure", "md_docs_2docs__overview.html#autotoc_md86", null ]
      ] ],
      [ "Documentation Coverage", "md_docs_2docs__overview.html#autotoc_md87", [
        [ "Source Files with Doxygen Comments", "md_docs_2docs__overview.html#autotoc_md88", null ],
        [ "Markdown Documentation", "md_docs_2docs__overview.html#autotoc_md89", null ]
      ] ],
      [ "Documentation Guidelines", "md_docs_2docs__overview.html#autotoc_md90", [
        [ "File Header", "md_docs_2docs__overview.html#autotoc_md91", null ],
        [ "Class Documentation", "md_docs_2docs__overview.html#autotoc_md92", null ],
        [ "Function/Method Documentation", "md_docs_2docs__overview.html#autotoc_md93", null ],
        [ "Inline Comments", "md_docs_2docs__overview.html#autotoc_md94", null ]
      ] ],
      [ "Maintenance", "md_docs_2docs__overview.html#autotoc_md95", [
        [ "Updating Documentation", "md_docs_2docs__overview.html#autotoc_md96", null ],
        [ "Documentation Testing", "md_docs_2docs__overview.html#autotoc_md97", null ]
      ] ],
      [ "Additional Resources", "md_docs_2docs__overview.html#autotoc_md98", null ]
    ] ],
    [ "MSBasic Implementation Status", "md_docs_2features.html", [
      [ "Part 1: Implementation Status (Todo-List Style)", "md_docs_2features.html#autotoc_md102", [
        [ "Core Architecture", "md_docs_2features.html#autotoc_md103", null ],
        [ "Expression Evaluation", "md_docs_2features.html#autotoc_md104", null ],
        [ "Standard Applesoft Commands", "md_docs_2features.html#autotoc_md105", [
          [ "Control Flow", "md_docs_2features.html#autotoc_md106", null ],
          [ "Variable and Data Management", "md_docs_2features.html#autotoc_md107", null ],
          [ "User-Defined Functions", "md_docs_2features.html#autotoc_md108", null ],
          [ "Input/Output", "md_docs_2features.html#autotoc_md109", null ],
          [ "Program Management", "md_docs_2features.html#autotoc_md110", null ],
          [ "File Operations", "md_docs_2features.html#autotoc_md111", null ],
          [ "Screen Control", "md_docs_2features.html#autotoc_md112", null ],
          [ "Graphics - Low Resolution", "md_docs_2features.html#autotoc_md113", null ],
          [ "Graphics - High Resolution", "md_docs_2features.html#autotoc_md114", null ],
          [ "Low-Level System Access", "md_docs_2features.html#autotoc_md115", null ],
          [ "Memory Management", "md_docs_2features.html#autotoc_md116", null ],
          [ "Error Handling", "md_docs_2features.html#autotoc_md117", null ],
          [ "Debugging and Timing", "md_docs_2features.html#autotoc_md118", null ],
          [ "Device Control", "md_docs_2features.html#autotoc_md119", null ]
        ] ],
        [ "ProDOS Commands", "md_docs_2features.html#autotoc_md120", [
          [ "File System Operations", "md_docs_2features.html#autotoc_md121", null ]
        ] ],
        [ "Built-in Functions", "md_docs_2features.html#autotoc_md122", [
          [ "Mathematical Functions", "md_docs_2features.html#autotoc_md123", null ],
          [ "String Functions", "md_docs_2features.html#autotoc_md124", null ],
          [ "System Functions", "md_docs_2features.html#autotoc_md125", null ]
        ] ],
        [ "Error Codes", "md_docs_2features.html#autotoc_md126", [
          [ "Standard Applesoft Error Codes", "md_docs_2features.html#autotoc_md127", null ],
          [ "ProDOS Error Codes", "md_docs_2features.html#autotoc_md128", null ],
          [ "Error Handling Memory Locations", "md_docs_2features.html#autotoc_md129", null ]
        ] ],
        [ "PEEK/POKE/CALL Address Support", "md_docs_2features.html#autotoc_md130", [
          [ "Keyboard and Input", "md_docs_2features.html#autotoc_md131", null ],
          [ "Memory Pointers", "md_docs_2features.html#autotoc_md132", null ],
          [ "Display Control", "md_docs_2features.html#autotoc_md133", null ],
          [ "Annunciator Outputs", "md_docs_2features.html#autotoc_md134", null ],
          [ "Graphics Memory", "md_docs_2features.html#autotoc_md135", null ],
          [ "Shape Tables", "md_docs_2features.html#autotoc_md136", null ],
          [ "Error Control", "md_docs_2features.html#autotoc_md137", null ],
          [ "System Calls (CALL addresses)", "md_docs_2features.html#autotoc_md138", null ]
        ] ],
        [ "Graphics Implementation", "md_docs_2features.html#autotoc_md139", [
          [ "No-Graphics Mode", "md_docs_2features.html#autotoc_md140", null ],
          [ "Graphics Mode - Display Window", "md_docs_2features.html#autotoc_md141", null ],
          [ "Graphics Mode - Build System", "md_docs_2features.html#autotoc_md142", null ],
          [ "Graphics Mode - Commands", "md_docs_2features.html#autotoc_md143", null ],
          [ "Graphics Mode - Text Mode Switching", "md_docs_2features.html#autotoc_md144", null ],
          [ "Graphics Mode - Graphics Buffer", "md_docs_2features.html#autotoc_md145", null ]
        ] ],
        [ "Font Integration", "md_docs_2features.html#autotoc_md146", [
          [ "Font Specifications", "md_docs_2features.html#autotoc_md147", null ],
          [ "Bundled Font Files", "md_docs_2features.html#autotoc_md148", null ],
          [ "CI/CD Integration", "md_docs_2features.html#autotoc_md149", null ],
          [ "Font Loading Implementation", "md_docs_2features.html#autotoc_md150", null ],
          [ "Character Rendering Implementation", "md_docs_2features.html#autotoc_md151", null ],
          [ "Text Rendering Implementation", "md_docs_2features.html#autotoc_md152", null ]
        ] ],
        [ "Remaining Work - Graphics", "md_docs_2features.html#autotoc_md153", null ],
        [ "Implementation Completeness", "md_docs_2features.html#autotoc_md154", [
          [ "Overall Score: 98%", "md_docs_2features.html#autotoc_md155", null ],
          [ "Feature Category Status", "md_docs_2features.html#autotoc_md156", null ]
        ] ]
      ] ],
      [ "Part 2: Implementation Details & Notes", "md_docs_2features.html#autotoc_md158", [
        [ "Graphics Architecture", "md_docs_2features.html#autotoc_md159", [
          [ "Mode Selection", "md_docs_2features.html#autotoc_md160", null ],
          [ "No-Graphics Mode Behavior", "md_docs_2features.html#autotoc_md161", null ],
          [ "Graphics Mode Usage Examples", "md_docs_2features.html#autotoc_md162", null ],
          [ "Graphics Implementation Details", "md_docs_2features.html#autotoc_md163", null ],
          [ "Platform Support", "md_docs_2features.html#autotoc_md164", null ],
          [ "CMake Build Options", "md_docs_2features.html#autotoc_md165", null ]
        ] ],
        [ "Variable Names", "md_docs_2features.html#autotoc_md166", null ],
        [ "Arrays", "md_docs_2features.html#autotoc_md167", null ],
        [ "DATA/READ/RESTORE", "md_docs_2features.html#autotoc_md168", null ],
        [ "Graphics", "md_docs_2features.html#autotoc_md169", null ],
        [ "Memory Model", "md_docs_2features.html#autotoc_md170", null ],
        [ "Output Behavior", "md_docs_2features.html#autotoc_md171", null ],
        [ "Device Control Behavior", "md_docs_2features.html#autotoc_md172", null ],
        [ "File I/O", "md_docs_2features.html#autotoc_md173", null ],
        [ "Testing", "md_docs_2features.html#autotoc_md174", null ],
        [ "Build and Development", "md_docs_2features.html#autotoc_md175", null ],
        [ "Known Limitations", "md_docs_2features.html#autotoc_md176", [
          [ "Hardware-Specific Features (By Design)", "md_docs_2features.html#autotoc_md177", null ],
          [ "ProDOS File Operations (Partially Implemented)", "md_docs_2features.html#autotoc_md178", null ]
        ] ],
        [ "Test Coverage Details", "md_docs_2features.html#autotoc_md179", [
          [ "Tests Present (75 total test programs)", "md_docs_2features.html#autotoc_md180", null ]
        ] ],
        [ "Future Enhancement Recommendations", "md_docs_2features.html#autotoc_md181", [
          [ "Priority 1: Enhance File I/O (Medium Effort)", "md_docs_2features.html#autotoc_md182", null ],
          [ "Priority 2: Complete Graphics Font Integration (Low Effort)", "md_docs_2features.html#autotoc_md183", null ],
          [ "Priority 3: Add More Examples (Low Effort)", "md_docs_2features.html#autotoc_md184", null ],
          [ "Priority 4: Documentation (Low Effort)", "md_docs_2features.html#autotoc_md185", null ]
        ] ],
        [ "Production Readiness", "md_docs_2features.html#autotoc_md186", null ],
        [ "Code Quality", "md_docs_2features.html#autotoc_md187", [
          [ "Security", "md_docs_2features.html#autotoc_md188", null ],
          [ "Best Practices", "md_docs_2features.html#autotoc_md189", null ]
        ] ],
        [ "Font Integration (Deep Dive)", "md_docs_2features.html#autotoc_md190", [
          [ "Automatic Font Fetching", "md_docs_2features.html#autotoc_md191", null ],
          [ "CI/CD Integration for Fonts", "md_docs_2features.html#autotoc_md192", null ],
          [ "Implementation Details", "md_docs_2features.html#autotoc_md193", null ],
          [ "Manual Installation", "md_docs_2features.html#autotoc_md194", null ],
          [ "Troubleshooting", "md_docs_2features.html#autotoc_md195", null ],
          [ "Version Control", "md_docs_2features.html#autotoc_md196", null ],
          [ "Alternative Approach", "md_docs_2features.html#autotoc_md197", null ]
        ] ]
      ] ]
    ] ],
    [ "MSBasic Documentation Implementation Summary", "md_docs_2IMPLEMENTATION__SUMMARY.html", [
      [ "Completed Tasks", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md199", [
        [ "1. Architecture Documentation", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md200", null ],
        [ "2. Doxygen Configuration", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md201", null ],
        [ "3. Doxygen Comments in Source Files", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md202", [
          [ "<tt>src/types.h</tt>", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md203", null ],
          [ "<tt>src/tokenizer.h</tt>", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md204", null ],
          [ "<tt>src/parser.h</tt>", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md205", null ],
          [ "<tt>src/variables.h</tt>", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md206", null ]
        ] ],
        [ "4. Documentation Infrastructure", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md207", [
          [ "<tt>docs/docs_overview.md</tt> (167 lines)", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md208", null ],
          [ "CMake Integration", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md209", null ],
          [ "<tt>.gitignore</tt> Updates", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md210", null ],
          [ "<tt>README.md</tt> Updates", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md211", null ]
        ] ],
        [ "5. Documentation Generation", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md212", null ]
      ] ],
      [ "Documentation Statistics", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md213", [
        [ "Coverage", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md214", null ],
        [ "Generated Output", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md215", null ]
      ] ],
      [ "How to Use the Documentation", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md216", [
        [ "Viewing HTML Documentation", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md217", null ],
        [ "Building PDF Documentation", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md218", null ],
        [ "Updating Documentation", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md219", null ]
      ] ],
      [ "Documentation Quality", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md220", [
        [ "Strengths", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md221", null ],
        [ "Areas for Future Improvement", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md222", null ]
      ] ],
      [ "Conclusion", "md_docs_2IMPLEMENTATION__SUMMARY.html#autotoc_md223", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classInterpreter.html#a45af7eb85abbe2a3c6bc25178e008745",
"classVariables.html#a46c30050683017ad2e4089a0970eb2a8",
"interpreter_8h.html#a63febca1a50971aae461dfceb448dfc5",
"namespaceErrorCode.html#a97950cb3cc899d6decab7d9fe03ab73e"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';