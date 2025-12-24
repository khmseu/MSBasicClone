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

# Logging
if(NOT DEFINED DOCS_LOG_DIR OR "${DOCS_LOG_DIR}" STREQUAL "")
  set(DOCS_LOG_DIR "${SOURCE_DIR}/build")
endif()
file(MAKE_DIRECTORY "${DOCS_LOG_DIR}")
set(_doxygen_log "${DOCS_LOG_DIR}/docs-doxygen.log")
set(_latex_log "${DOCS_LOG_DIR}/docs-latex.log")

# Options
if(NOT DEFINED DOCS_CLEAN_OUTPUT)
  set(DOCS_CLEAN_OUTPUT ON)
endif()
if(NOT DEFINED DOCS_ENABLE_DOT)
  set(DOCS_ENABLE_DOT OFF)
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
unset(_latex_cmd CACHE)
set(_latex_cmd "")
string(TOLOWER "${DOCS_LATEX_ENGINE}" _engine)

# Make sure `find_program()` consults the environment PATH.
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH TRUE)

# Some CMake find_* commands can cache results across runs in unexpected ways,
# especially when invoked via `cmake -P` from different working directories.
# Make sure we re-search after the user installs TeX.
set(_latex_search_paths
  /usr/bin
  /bin
  /usr/local/bin
)

function(_docs_try_set_latex_cmd _name)
  if(_latex_cmd)
    return()
  endif()

  foreach(_p IN LISTS _latex_search_paths)
    if(EXISTS "${_p}/${_name}")
      set(_latex_cmd "${_p}/${_name}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

if(DOCS_BUILD_PDF)
  # Doxygen/LaTeX generation may need epstopdf (EPS -> PDF conversion),
  # especially when diagrams are emitted as EPS.
  find_program(_epstopdf_cmd NAMES epstopdf)
  if(NOT _epstopdf_cmd)
    message(FATAL_ERROR
      "DOCS_BUILD_PDF=ON but 'epstopdf' was not found.\n"
      "This tool is required to convert EPS images for the LaTeX/PDF pipeline.\n"
      "On Debian/Ubuntu: sudo apt-get install texlive-font-utils\n"
      "Or disable the PDF step by configuring with: -DDOCS_BUILD_PDF=OFF"
    )
  endif()

  # Prefer explicit, well-known locations first to avoid any find_program()
  # quirks when invoked via build tools.
  if(_engine STREQUAL "xelatex")
    _docs_try_set_latex_cmd("xelatex")
  elseif(_engine STREQUAL "lualatex")
    _docs_try_set_latex_cmd("lualatex")
  elseif(_engine STREQUAL "pdflatex")
    _docs_try_set_latex_cmd("pdflatex")
  endif()

  if(_engine STREQUAL "xelatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES xelatex HINTS ${_latex_search_paths})
    endif()
  elseif(_engine STREQUAL "lualatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES lualatex HINTS ${_latex_search_paths})
    endif()
  elseif(_engine STREQUAL "pdflatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES pdflatex HINTS ${_latex_search_paths})
    endif()
  else()
    message(WARNING "Unknown DOCS_LATEX_ENGINE='${DOCS_LATEX_ENGINE}', trying xelatex")
    _docs_try_set_latex_cmd("xelatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES xelatex HINTS ${_latex_search_paths})
    endif()
  endif()

  if(NOT _latex_cmd)
    _docs_try_set_latex_cmd("xelatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES xelatex HINTS ${_latex_search_paths})
    endif()
  endif()
  if(NOT _latex_cmd)
    _docs_try_set_latex_cmd("lualatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES lualatex HINTS ${_latex_search_paths})
    endif()
  endif()
  if(NOT _latex_cmd)
    _docs_try_set_latex_cmd("pdflatex")
    if(NOT _latex_cmd)
      find_program(_latex_cmd NAMES pdflatex HINTS ${_latex_search_paths})
    endif()
  endif()

  if(NOT _latex_cmd)
    message(FATAL_ERROR
      "No LaTeX engine found (tried xelatex, lualatex, pdflatex).\n"
      "PATH=$ENV{PATH}\n"
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

# Stamp the current project version into the Doxygen config so it appears in
# generated pages (PROJECT_NUMBER). The version is passed from the parent
# CMake (MSBASIC_VERSION_STRING) when invoking this script.
if(DEFINED MSBASIC_VERSION_STRING)
  file(APPEND "${_doxy_out}" "PROJECT_NUMBER = ${MSBASIC_VERSION_STRING}\n")
else()
  message(STATUS "MSBASIC_VERSION_STRING not defined; PROJECT_NUMBER will remain as set in Doxyfile")
endif()

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
message(STATUS "Running Doxygen (log: ${_doxygen_log})")
execute_process(
  COMMAND "${DOXYGEN_EXECUTABLE}" "${_doxy_out}"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _doxygen_rv
  OUTPUT_FILE "${_doxygen_log}"
  ERROR_FILE "${_doxygen_log}"
)

if(NOT _doxygen_rv EQUAL 0)
  _print_tail("${_doxygen_log}" 80)
  message(FATAL_ERROR "Doxygen failed with exit code ${_doxygen_rv}")
endif()

# Copy repository assets into the generated HTML output so image paths
# used in markdown like `assets/logo/rbr.png` resolve on the site.
if(EXISTS "${SOURCE_DIR}/assets")
  set(_html_assets_dir "${_output_dir}/html/assets")
  message(STATUS "Copying repository assets to docs output: ${SOURCE_DIR}/assets -> ${_html_assets_dir}")
  # Ensure target directory exists and perform a directory copy
  file(MAKE_DIRECTORY "${_html_assets_dir}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SOURCE_DIR}/assets" "${_html_assets_dir}")
endif()

# Doxygen's LaTeX output is primarily tailored for pdfTeX/LuaTeX.
# When using XeLaTeX, guard pdfTeX-only primitives that can appear in refman.tex.
if(DOCS_BUILD_PDF AND _engine STREQUAL "xelatex")
  if(EXISTS "${_latex_dir}/refman.tex")
    file(READ "${_latex_dir}/refman.tex" _refman_tex)
    set(_refman_tex_patched "${_refman_tex}")

    # XeTeX does not define \pdfminorversion; make it conditional.
    # Keeps behavior for engines that do define it, and avoids a hard error on XeLaTeX.
    string(REPLACE "\\pdfminorversion=7" "\\ifdefined\\pdfminorversion\\pdfminorversion=7\\fi" _refman_tex_patched "${_refman_tex_patched}")
    string(REPLACE "\\pdfminorversion =7" "\\ifdefined\\pdfminorversion\\pdfminorversion=7\\fi" _refman_tex_patched "${_refman_tex_patched}")
    string(REPLACE "\\pdfminorversion = 7" "\\ifdefined\\pdfminorversion\\pdfminorversion=7\\fi" _refman_tex_patched "${_refman_tex_patched}")

    # XeTeX also does not define \pdfsuppresswarningpagegroup; guard it similarly.
    string(REPLACE "\\pdfsuppresswarningpagegroup=1" "\\ifdefined\\pdfsuppresswarningpagegroup\\pdfsuppresswarningpagegroup=1\\fi" _refman_tex_patched "${_refman_tex_patched}")
    string(REPLACE "\\pdfsuppresswarningpagegroup =1" "\\ifdefined\\pdfsuppresswarningpagegroup\\pdfsuppresswarningpagegroup=1\\fi" _refman_tex_patched "${_refman_tex_patched}")
    string(REPLACE "\\pdfsuppresswarningpagegroup = 1" "\\ifdefined\\pdfsuppresswarningpagegroup\\pdfsuppresswarningpagegroup=1\\fi" _refman_tex_patched "${_refman_tex_patched}")

    # Some TeX distributions ship a newunicodechar version without \nuc@check.
    # Doxygen's refman.tex defines \doxynewunicodechar in terms of \nuc@check, so
    # provide a small compatibility shim to avoid a hard XeLaTeX failure.
    string(REPLACE
      "  \\usepackage{newunicodechar}\n  \\makeatletter\n    \\def\\doxynewunicodechar"
      "  \\usepackage{newunicodechar}\n  \\makeatletter\n  \\@ifundefined{nuc@check}{\\def\\nuc@check{\\@tempswatrue}}{}\n  \\@ifundefined{nuc@emptyargerr}{\\def\\nuc@emptyargerr{}}{}\n    \\def\\doxynewunicodechar"
      _refman_tex_patched
      "${_refman_tex_patched}"
    )

    if(NOT "${_refman_tex_patched}" STREQUAL "${_refman_tex}")
      file(WRITE "${_latex_dir}/refman.tex" "${_refman_tex_patched}")
    endif()
  endif()
endif()

if(NOT EXISTS "${_latex_dir}/Makefile")
  message(FATAL_ERROR "Expected LaTeX output at ${_latex_dir} (missing Makefile)")
endif()

if(DOCS_BUILD_PDF)
  # Build PDF; if it fails, print log tail.
  message(STATUS "Building PDF with LATEX_CMD=${_latex_cmd} (log: ${_latex_log})")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E chdir "${_latex_dir}" make "LATEX_CMD=${_latex_cmd}"
    RESULT_VARIABLE _make_rv
    OUTPUT_FILE "${_latex_log}"
    ERROR_FILE "${_latex_log}"
  )

  if(NOT _make_rv EQUAL 0)
    _print_tail("${_latex_log}" 80)
    _print_tail("${_latex_dir}/refman.log" 80)
    message(FATAL_ERROR "LaTeX build failed with exit code ${_make_rv}")
  endif()

  message(STATUS "Docs built successfully: ${_latex_dir}/refman.pdf")
  message(STATUS "Logs saved: ${_doxygen_log}; ${_latex_log}")
else()
  message(STATUS "PDF build skipped (DOCS_BUILD_PDF=OFF). HTML/LaTeX outputs are under ${_output_dir}.")
  message(STATUS "Log saved: ${_doxygen_log}")
endif()
