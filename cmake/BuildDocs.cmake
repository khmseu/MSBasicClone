cmake_minimum_required(VERSION 3.20)

# Inputs (must be passed via -D...)
if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()
if(NOT DEFINED DOXYGEN_EXECUTABLE)
  message(FATAL_ERROR "DOXYGEN_EXECUTABLE is required")
endif()

set(_output_dir "${SOURCE_DIR}/docs/api")
set(_latex_dir "${_output_dir}/latex")

# Options
if(NOT DEFINED DOCS_CLEAN_OUTPUT)
  set(DOCS_CLEAN_OUTPUT ON)
endif()
if(NOT DEFINED DOCS_ENABLE_DOT)
  set(DOCS_ENABLE_DOT ON)
endif()
if(NOT DEFINED DOCS_BUILD_PDF)
  set(DOCS_BUILD_PDF ON)
endif()
if(NOT DEFINED DOCS_LATEX_ENGINE OR "${DOCS_LATEX_ENGINE}" STREQUAL "")
  set(DOCS_LATEX_ENGINE "xelatex")
endif()

function(_print_tail file_path max_lines)
  if(NOT EXISTS "${file_path}")
    message(STATUS "(log missing) ${file_path}")
    return()
  endif()

  file(READ "${file_path}" _content)
  string(REPLACE "\r\n" "\n" _content "${_content}")
  string(REPLACE "\r" "\n" _content "${_content}")
  string(REPLACE "\n" ";" _lines "${_content}")

  list(LENGTH _lines _len)
  if(_len LESS_EQUAL 1)
    message(STATUS "${_content}")
    return()
  endif()

  math(EXPR _start "${_len} - ${max_lines}")
  if(_start LESS 0)
    set(_start 0)
  endif()

  message(STATUS "----- tail(${max_lines}): ${file_path} -----")
  foreach(_i RANGE ${_start} ${_len})
    if(_i LESS _len)
      list(GET _lines ${_i} _line)
      message(STATUS "${_line}")
    endif()
  endforeach()
  message(STATUS "----- end tail -----")
endfunction()

# Pick LaTeX engine (switch) with fallback.
set(_latex_cmd "")
string(TOLOWER "${DOCS_LATEX_ENGINE}" _engine)

if(DOCS_BUILD_PDF)
  if(_engine STREQUAL "xelatex")
    find_program(_latex_cmd xelatex)
  elseif(_engine STREQUAL "lualatex")
    find_program(_latex_cmd lualatex)
  elseif(_engine STREQUAL "pdflatex")
    find_program(_latex_cmd pdflatex)
  else()
    message(WARNING "Unknown DOCS_LATEX_ENGINE='${DOCS_LATEX_ENGINE}', trying xelatex")
    find_program(_latex_cmd xelatex)
  endif()

  if(NOT _latex_cmd)
    find_program(_latex_cmd xelatex)
  endif()
  if(NOT _latex_cmd)
    find_program(_latex_cmd lualatex)
  endif()
  if(NOT _latex_cmd)
    find_program(_latex_cmd pdflatex)
  endif()

  if(NOT _latex_cmd)
    message(FATAL_ERROR
      "No LaTeX engine found (tried xelatex, lualatex, pdflatex).\n"
      "To build the PDF on Debian/Ubuntu: sudo apt-get install texlive-xetex texlive-luatex texlive-latex-recommended texlive-latex-extra texlive-fonts-recommended\n"
      "Or disable the PDF step by configuring with: -DDOCS_BUILD_PDF=OFF"
    )
  endif()
endif()

# Graphviz (upgrade guardrails): optional, but warn about known-bad versions.
if(DOCS_ENABLE_DOT)
  find_program(_dot_cmd dot)
  if(NOT _dot_cmd)
    message(FATAL_ERROR "DOCS_ENABLE_DOT=ON but Graphviz 'dot' not found")
  endif()

  execute_process(
    COMMAND "${_dot_cmd}" -V
    RESULT_VARIABLE _dot_rv
    OUTPUT_VARIABLE _dot_out
    ERROR_VARIABLE _dot_err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )

  # dot -V writes to stderr on many versions
  if(NOT "${_dot_err}" STREQUAL "")
    set(_dot_ver "${_dot_err}")
  else()
    set(_dot_ver "${_dot_out}")
  endif()

  message(STATUS "Graphviz: ${_dot_ver}")
  if(_dot_ver MATCHES "[Vv]ersion[ ]+2\\.43\\.0")
    message(WARNING "Graphviz 2.43.0 is known to crash on some Doxygen-generated graphs; consider upgrading Graphviz or keep DOCS_ENABLE_DOT=OFF")
  endif()
endif()

# Generate a temporary Doxyfile so DOT can be toggled without editing the repo file.
set(_doxy_in "${SOURCE_DIR}/Doxyfile")
set(_doxy_out "${SOURCE_DIR}/build/Doxyfile.docs")

# IMPORTANT: Do not rewrite the repository Doxyfile content.
#
# The upstream Doxyfile uses line continuations with a trailing '\\' for
# list-valued settings (e.g. ABBREVIATE_BRIEF, INPUT, FILE_PATTERNS, EXCLUDE).
# Any line-based rewrite in CMake risks corrupting those continuations and
# producing a malformed config (leading to Doxygen "unknown tag" warnings).
#
# Instead, generate a tiny config that includes the repo Doxyfile and then
# overrides only the few keys we need.
file(WRITE "${_doxy_out}" "@INCLUDE = ${_doxy_in}\n")

# Ensure docs always go to the intended output directory.
file(APPEND "${_doxy_out}" "OUTPUT_DIRECTORY = ${_output_dir}\n")

if(DOCS_ENABLE_DOT)
  file(APPEND "${_doxy_out}" "HAVE_DOT = YES\n")
  file(APPEND "${_doxy_out}" "DOT_IMAGE_FORMAT = png\n")
  if(DEFINED _dot_cmd AND EXISTS "${_dot_cmd}")
    get_filename_component(_dot_dir "${_dot_cmd}" DIRECTORY)
    if(NOT "${_dot_dir}" STREQUAL "")
      file(APPEND "${_doxy_out}" "DOT_PATH = ${_dot_dir}\n")
    endif()
  endif()
else()
  file(APPEND "${_doxy_out}" "HAVE_DOT = NO\n")
endif()

# If we're not building a PDF, avoid TeX toolchain dependencies during the
# Doxygen phase (notably epstopdf). Doxygen only needs epstopdf when
# USE_PDFLATEX=YES to convert EPS -> PDF.
if(NOT DOCS_BUILD_PDF)
  file(APPEND "${_doxy_out}" "USE_PDFLATEX = NO\n")
endif()

# Clean output dir before generating (simple + avoids mixed stale artifacts)
if(DOCS_CLEAN_OUTPUT)
  message(STATUS "Removing generated docs at ${_output_dir}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E rm -rf "${_output_dir}" RESULT_VARIABLE _rm_rv)
  if(NOT _rm_rv EQUAL 0)
    message(FATAL_ERROR "Failed to remove ${_output_dir}")
  endif()
endif()

# Run Doxygen
message(STATUS "Running Doxygen...")
execute_process(
  COMMAND "${DOXYGEN_EXECUTABLE}" "${_doxy_out}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _doxygen_rv
)

if(NOT _doxygen_rv EQUAL 0)
  message(FATAL_ERROR "Doxygen failed with exit code ${_doxygen_rv}")
endif()

if(NOT EXISTS "${_latex_dir}/Makefile")
  message(FATAL_ERROR "Expected LaTeX output at ${_latex_dir} (missing Makefile)")
endif()

if(DOCS_BUILD_PDF)
  # Build PDF; if it fails, print log tail.
  message(STATUS "Building PDF with LATEX_CMD=${_latex_cmd}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E chdir "${_latex_dir}" make LATEX_CMD="${_latex_cmd}"
    RESULT_VARIABLE _make_rv
  )

  if(NOT _make_rv EQUAL 0)
    _print_tail("${_latex_dir}/refman.log" 80)
    message(FATAL_ERROR "LaTeX build failed with exit code ${_make_rv}")
  endif()

  message(STATUS "Docs built successfully: ${_latex_dir}/refman.pdf")
else()
  message(STATUS "PDF build skipped (DOCS_BUILD_PDF=OFF). HTML/LaTeX outputs are under ${_output_dir}.")
endif()
