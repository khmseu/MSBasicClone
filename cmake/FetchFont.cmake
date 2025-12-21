# FetchFont.cmake
# Manage Ultimate Apple II Font and charset map
# These files are bundled in the repository but can be refreshed from upstream

function(fetch_apple2_font)
    set(FONT_DIR "${CMAKE_SOURCE_DIR}/assets/fonts")
    set(CHARSET_DIR "${FONT_DIR}/charset")
    set(FONT_FILE "${FONT_DIR}/PrintChar21.ttf")
    set(CHARSET_FILE "${CHARSET_DIR}/index.html")
    set(LICENSE_FILE "${FONT_DIR}/FreeLicense.txt")
    set(FONT_PACKAGE_URL "https://www.kreativekorp.com/swdownload/fonts/retro/pr.zip")
    set(CHARSET_URL "https://www.kreativekorp.com/charset/map/mousetext/")

    # Allow override of URLs via CMake variable
    if(DEFINED APPLE2_FONT_PACKAGE_URL)
        set(FONT_PACKAGE_URL "${APPLE2_FONT_PACKAGE_URL}")
    endif()

    if(DEFINED APPLE2_CHARSET_URL)
        set(CHARSET_URL "${APPLE2_CHARSET_URL}")
    endif()

    # Option to force refresh bundled files
    option(REFRESH_BUNDLED_FONTS "Force refresh of bundled font files from upstream" OFF)

    # Create fonts directory if it doesn't exist
    if(NOT EXISTS "${FONT_DIR}")
        file(MAKE_DIRECTORY "${FONT_DIR}")
        message(STATUS "Created fonts directory: ${FONT_DIR}")
    endif()

    # Create charset subdirectory if it doesn't exist
    if(NOT EXISTS "${CHARSET_DIR}")
        file(MAKE_DIRECTORY "${CHARSET_DIR}")
        message(STATUS "Created charset directory: ${CHARSET_DIR}")
    endif()

    # Check if all bundled files exist
    set(ALL_FILES_PRESENT TRUE)
    if(NOT EXISTS "${FONT_FILE}" OR NOT EXISTS "${CHARSET_FILE}" OR NOT EXISTS "${LICENSE_FILE}")
        set(ALL_FILES_PRESENT FALSE)
    endif()

    # Refresh logic: download if files are missing or refresh is forced
    if(NOT ALL_FILES_PRESENT OR REFRESH_BUNDLED_FONTS)
        if(REFRESH_BUNDLED_FONTS)
            message(STATUS "REFRESH_BUNDLED_FONTS is ON - refreshing font files from upstream...")
        else()
            message(STATUS "Font files missing - downloading from upstream...")
        endif()

        # Download the font package (contains font and license)
        set(TEMP_ZIP "${CMAKE_BINARY_DIR}/apple2_font.zip")
        message(STATUS "Downloading font package from ${FONT_PACKAGE_URL}...")

        # Find wget
        find_program(WGET_EXECUTABLE wget)

        if(WGET_EXECUTABLE)
            # Use wget to download (server requires browser-like User-Agent)
            execute_process(
                COMMAND ${WGET_EXECUTABLE} -O "${TEMP_ZIP}"
                    --user-agent="Mozilla/5.0"
                    -e robots=off
                    ${FONT_PACKAGE_URL}
                RESULT_VARIABLE WGET_RESULT
                OUTPUT_VARIABLE WGET_OUTPUT
                ERROR_VARIABLE WGET_ERROR
            )

            if(WGET_RESULT EQUAL 0 AND EXISTS "${TEMP_ZIP}")
                message(STATUS "Successfully downloaded font package")

                # Extract the required files
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E tar xf "${TEMP_ZIP}"
                    WORKING_DIRECTORY "${FONT_DIR}"
                    RESULT_VARIABLE EXTRACT_RESULT
                )

                if(EXTRACT_RESULT EQUAL 0)
                    message(STATUS "Successfully extracted font files")
                else()
                    message(WARNING "Failed to extract font package")
                endif()

                # Clean up zip file
                file(REMOVE "${TEMP_ZIP}")
            else()
                message(WARNING "Failed to download font package with wget")
                message(WARNING "wget result: ${WGET_RESULT}")
                message(WARNING "You can manually download from:")
                message(WARNING "  https://www.kreativekorp.com/software/fonts/apple2/")
                message(WARNING "or directly from")
                message(WARNING "  ${FONT_PACKAGE_URL}")
                message(WARNING "And extract PrintChar21.ttf and FreeLicense.txt to: ${FONT_DIR}")

                # Clean up partial download if it exists
                if(EXISTS "${TEMP_ZIP}")
                    file(REMOVE "${TEMP_ZIP}")
                endif()
            endif()
        else()
            message(WARNING "wget not found - cannot download font package")
            message(WARNING "The server requires a browser-like User-Agent header")
            message(WARNING "Please install wget or manually download from:")
            message(WARNING "  https://www.kreativekorp.com/software/fonts/apple2/")
            message(WARNING "or directly from")
            message(WARNING "  ${FONT_PACKAGE_URL}")
            message(WARNING "And extract PrintChar21.ttf and FreeLicense.txt to: ${FONT_DIR}")
        endif()

        # Download charset map using wget to get directory with all images
        if(NOT EXISTS "${CHARSET_FILE}" OR REFRESH_BUNDLED_FONTS)
            message(STATUS "Downloading mousetext charset map from ${CHARSET_URL}...")

            # Find wget
            find_program(WGET_EXECUTABLE wget)

            if(WGET_EXECUTABLE)
                # Use wget to download the directory with all images
                # Flags: -p (page requisites), -np (no parent), -nd (no directories),
                #        --include-directories (limit to path), -e robots=off (ignore robots.txt)
                execute_process(
                    COMMAND ${WGET_EXECUTABLE} -p -np -nd
                        --include-directories=/charset/map/mousetext/
                        -e robots=off
                        ${CHARSET_URL}
                    WORKING_DIRECTORY "${CHARSET_DIR}"
                    RESULT_VARIABLE WGET_RESULT
                    OUTPUT_VARIABLE WGET_OUTPUT
                    ERROR_VARIABLE WGET_ERROR
                )

                if(WGET_RESULT EQUAL 0)
                    message(STATUS "Successfully downloaded mousetext charset map with images")
                else()
                    message(WARNING "Failed to download charset map with wget")
                    message(WARNING "wget result: ${WGET_RESULT}")
                    message(WARNING "wget output: ${WGET_OUTPUT}")
                    message(WARNING "wget error: ${WGET_ERROR}")
                    message(WARNING "You can manually download from: ${CHARSET_URL}")
                endif()
            else()
                message(WARNING "wget not found - cannot download charset map directory")
                message(WARNING "Please install wget or manually download from:")
                message(WARNING "  ${CHARSET_URL}")
                message(WARNING "Save all files to: ${CHARSET_DIR}")
            endif()
        endif()
    else()
        # All files present and no refresh requested
        message(STATUS "Font files are bundled and present:")
        message(STATUS "  - ${FONT_FILE}")
        message(STATUS "  - ${CHARSET_FILE}")
        message(STATUS "  - ${LICENSE_FILE}")
        message(STATUS "To refresh from upstream, configure with -DREFRESH_BUNDLED_FONTS=ON")
    endif()

    # Set a cache variable to indicate font availability for use by build system
    # This can be checked by other parts of the build or by external projects
    if(EXISTS "${FONT_FILE}")
        set(APPLE2_FONT_AVAILABLE TRUE CACHE BOOL "Apple II font is available" FORCE)
    else()
        set(APPLE2_FONT_AVAILABLE FALSE CACHE BOOL "Apple II font is available" FORCE)
    endif()

endfunction()
