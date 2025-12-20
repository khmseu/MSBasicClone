# FetchFont.cmake
# Automatically download Ultimate Apple II Font and charset map

function(fetch_apple2_font)
    set(FONT_DIR "${CMAKE_SOURCE_DIR}/assets/fonts")
    set(FONT_FILE "${FONT_DIR}/PrintChar21.ttf")
    set(CHARSET_FILE "${FONT_DIR}/apple2-charset.html")
    
    # Allow override of font URL via CMake variable
    if(NOT DEFINED APPLE2_FONT_URL)
        set(APPLE2_FONT_URL "https://www.kreativekorp.com/swdownload/fonts/apple2/PrintChar21.ttf")
    endif()
    
    if(NOT DEFINED APPLE2_CHARSET_URL)
        set(APPLE2_CHARSET_URL "https://www.kreativekorp.com/charset/map/apple2/")
    endif()
    
    # Create fonts directory if it doesn't exist
    if(NOT EXISTS "${FONT_DIR}")
        file(MAKE_DIRECTORY "${FONT_DIR}")
        message(STATUS "Created fonts directory: ${FONT_DIR}")
    endif()
    
    # Check if font already exists
    if(EXISTS "${FONT_FILE}")
        message(STATUS "Ultimate Apple II Font already present: ${FONT_FILE}")
    else()
        message(STATUS "Attempting to download Ultimate Apple II Font...")
        
        # Try to download the font
        file(DOWNLOAD 
            "${APPLE2_FONT_URL}"
            "${FONT_FILE}"
            STATUS DOWNLOAD_STATUS
            TIMEOUT 30
            TLS_VERIFY ON
        )
        
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        list(GET DOWNLOAD_STATUS 1 STATUS_MESSAGE)
        
        if(STATUS_CODE EQUAL 0)
            message(STATUS "Successfully downloaded Ultimate Apple II Font")
        else()
            message(WARNING "Failed to download font: ${STATUS_MESSAGE}")
            message(WARNING "Font URL: ${APPLE2_FONT_URL}")
            message(WARNING "You can manually download the font from:")
            message(WARNING "  https://www.kreativekorp.com/software/fonts/apple2/")
            message(WARNING "And place it at: ${FONT_FILE}")
            message(WARNING "Or set APPLE2_FONT_URL CMake variable to an alternative source")
            message(STATUS "Build will continue without the font (using Raylib default)")
            
            # Clean up partial download if it exists
            if(EXISTS "${FONT_FILE}")
                file(REMOVE "${FONT_FILE}")
            endif()
        endif()
    endif()
    
    # Check if charset map already exists
    if(EXISTS "${CHARSET_FILE}")
        message(STATUS "Apple II charset map already present: ${CHARSET_FILE}")
    else()
        message(STATUS "Attempting to download Apple II charset map...")
        
        # Try to download the charset map
        file(DOWNLOAD 
            "${APPLE2_CHARSET_URL}"
            "${CHARSET_FILE}"
            STATUS DOWNLOAD_STATUS
            TIMEOUT 30
            TLS_VERIFY ON
        )
        
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        list(GET DOWNLOAD_STATUS 1 STATUS_MESSAGE)
        
        if(STATUS_CODE EQUAL 0)
            message(STATUS "Successfully downloaded Apple II charset map")
        else()
            message(WARNING "Failed to download charset map: ${STATUS_MESSAGE}")
            message(WARNING "Charset URL: ${APPLE2_CHARSET_URL}")
            message(WARNING "You can manually download the charset map from the URL above")
            message(WARNING "Or set APPLE2_CHARSET_URL CMake variable to an alternative source")
            message(STATUS "Build will continue without the charset map")
            
            # Clean up partial download if it exists
            if(EXISTS "${CHARSET_FILE}")
                file(REMOVE "${CHARSET_FILE}")
            endif()
        endif()
    endif()
    
    # Set a cache variable to indicate font availability for use by build system
    # This can be checked by other parts of the build or by external projects
    if(EXISTS "${FONT_FILE}")
        set(APPLE2_FONT_AVAILABLE TRUE CACHE BOOL "Apple II font is available" FORCE)
    else()
        set(APPLE2_FONT_AVAILABLE FALSE CACHE BOOL "Apple II font is available" FORCE)
    endif()
    
endfunction()
