/**
 * @file tape_manager.cpp
 * @brief Implementation of cassette tape emulation for data persistence
 * 
 * This file implements the TapeManager class which provides tape-like
 * sequential storage for BASIC arrays and shape tables, matching
 * Applesoft BASIC's cassette tape operations.
 * 
 * Tape format:
 * - Sequential records with SIZE|DATA structure
 * - 4-byte size header (uint32_t) followed by data
 * - Position maintained across operations
 * - Rewind only on explicit tape change
 * 
 * Supported operations:
 * - STORE: Save array to tape (appends at current position)
 * - RECALL: Load array from tape (reads from current position)
 * - SHLOAD: Load shape table from tape
 * - TAPE "filename": Set/change tape file (rewinds)
 * - TAPE: Display current tape file
 * 
 * Platform-specific file selectors:
 * - Windows: Native file dialog via COM API
 * - macOS: osascript with AppleScript
 * - Linux: zenity or kdialog (GTK/KDE)
 * 
 * Safety features:
 * - Maximum record size limit (1 MB) prevents corruption issues
 * - Automatic close on write-to-read mode transition
 * - Position tracking for sequential access
 */

#include "tape_manager.h"
#include <stdexcept>
#include <cstring>
#include <iostream>

// Maximum record size (1 MB) - prevents excessive memory allocation from corrupt tapes
constexpr uint32_t MAX_RECORD_SIZE = 1024 * 1024;

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <commdlg.h>
#elif defined(PLATFORM_MACOS)
#include <cstdlib>
#elif defined(PLATFORM_LINUX)
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#endif

TapeManager::TapeManager() : readMode_(false), position_(0) {}

/**
 * @brief Destructor - closes tape file if open
 */
TapeManager::~TapeManager() {
    close();
}

/**
 * @brief Set the current tape file
 * 
 * Changes the tape file path and resets position to start. Closes any
 * currently open tape file.
 * 
 * Usage in BASIC:
 *   TAPE "MYDATA.TAP"  (sets current tape file)
 * 
 * @param filename Path to tape file
 */
void TapeManager::setTapeFile(const std::string& filename) {
    if (isOpen()) {
        close();
    }
    currentTape_ = filename;
    position_ = 0;
}

/**
 * @brief Open tape file for reading
 * 
 * Opens the current tape file in read mode for RECALL and SHLOAD operations.
 * If already open in read mode, keeps current position. If open in write mode,
 * closes and reopens in read mode.
 * 
 * Position tracking allows sequential reads across multiple RECALL commands
 * without rewinding.
 * 
 * @throws std::runtime_error if no tape loaded or file cannot be opened
 */
void TapeManager::openForRead() {
    if (currentTape_.empty()) {
        throw std::runtime_error("NO TAPE LOADED");
    }
    
    // If already open in read mode, keep position
    if (isOpen() && readMode_) {
        return;
    }
    
    // If open in write mode, close and reopen in read mode
    if (isOpen() && !readMode_) {
        close();
    }
    
    // Open for reading only if not already open
    if (!isOpen()) {
        file_.open(currentTape_, std::ios::in | std::ios::binary);
        if (!file_) {
            throw std::runtime_error("TAPE READ ERROR");
        }
        readMode_ = true;
        position_ = 0;
    }
}

/**
 * @brief Open tape file for writing
 * 
 * Opens the current tape file in write mode for STORE operations. If already
 * open in write mode, keeps current position. If open in read mode, closes
 * and reopens in write mode.
 * 
 * Write operations append at the current position, allowing sequential writes
 * across multiple STORE commands.
 * 
 * @throws std::runtime_error if no tape loaded or file cannot be opened
 */
void TapeManager::openForWrite() {
    if (currentTape_.empty()) {
        throw std::runtime_error("NO TAPE LOADED");
    }
    
    // If already open in write mode, keep position
    if (isOpen() && !readMode_) {
        return;
    }
    
    // If open in read mode, close and reopen in write mode
    if (isOpen() && readMode_) {
        close();
    }
    
    // Open for writing only if not already open
    if (!isOpen()) {
        // Open in read-write mode to allow positioning
        file_.open(currentTape_, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        if (!file_) {
            throw std::runtime_error("TAPE WRITE ERROR");
        }
        readMode_ = false;
        
        // Get current position for append mode
        file_.seekp(0, std::ios::end);
        position_ = file_.tellp();
    }
}

/**
 * @brief Close the tape file
 * 
 * Closes the current tape file and resets position to 0. Safe to call
 * even if no file is open.
 */
void TapeManager::close() {
    if (file_.is_open()) {
        file_.close();
    }
    position_ = 0;
}

/**
 * @brief Rewind tape to beginning
 * 
 * Resets file position to the start of the tape. Used when starting over
 * with a sequence of reads or writes. No-op if tape is not open.
 */
void TapeManager::rewind() {
    if (!isOpen()) {
        return;
    }
    
    if (readMode_) {
        file_.clear();
        file_.seekg(0, std::ios::beg);
    } else {
        file_.clear();
        file_.seekp(0, std::ios::beg);
    }
    position_ = 0;
}

void TapeManager::writeRecord(const std::vector<uint8_t>& data) {
    if (!isOpen() || readMode_) {
        throw std::runtime_error("TAPE NOT OPEN FOR WRITING");
    }
    
    // Write record size as 4-byte integer in native byte order
    // Note: For cross-platform compatibility, tape files should be used on the same architecture
    // or implement explicit little-endian serialization
    uint32_t size = static_cast<uint32_t>(data.size());
    file_.write(reinterpret_cast<const char*>(&size), sizeof(size));
    
    // Write record data
    file_.write(reinterpret_cast<const char*>(data.data()), data.size());
    
    if (!file_) {
        throw std::runtime_error("TAPE WRITE ERROR");
    }
    
    // Flush to ensure data is written to disk
    file_.flush();
    
    updatePosition();
}

std::vector<uint8_t> TapeManager::readRecord() {
    if (!isOpen() || !readMode_) {
        throw std::runtime_error("TAPE NOT OPEN FOR READING");
    }
    
    // Read record size
    uint32_t size;
    file_.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    if (!file_ || file_.eof()) {
        throw std::runtime_error("END OF TAPE");
    }
    
    if (size == 0 || size > MAX_RECORD_SIZE) {
        throw std::runtime_error("INVALID TAPE FORMAT");
    }
    
    // Read record data
    std::vector<uint8_t> data(size);
    file_.read(reinterpret_cast<char*>(data.data()), size);
    
    if (!file_ || static_cast<size_t>(file_.gcount()) != size) {
        throw std::runtime_error("TAPE READ ERROR");
    }
    
    updatePosition();
    return data;
}

/**
 * @brief Check if end of tape has been reached
 * 
 * Returns true if tape is open for reading and EOF has been reached.
 * Returns false if not in read mode or tape not open.
 * 
 * @return true if at end of tape, false otherwise
 */
bool TapeManager::isEndOfTape() const {
    if (!isOpen() || !readMode_) {
        return false;
    }
    return file_.eof();
}

/**
 * @brief Update cached position from file handle
 * 
 * Reads the current file position (tellg for read mode, tellp for write mode)
 * and caches it in position_ member. Called after read/write operations to
 * maintain accurate position tracking.
 */
void TapeManager::updatePosition() {
    if (readMode_) {
        position_ = file_.tellg();
    } else {
        position_ = file_.tellp();
    }
}

std::string TapeManager::showFileSelector(const std::string& title) {
#if defined(PLATFORM_WINDOWS)
    char filename[MAX_PATH] = "";
    
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Tape Files (*.tap)\0*.tap\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
    
#elif defined(PLATFORM_MACOS)
    // Use osascript to show native file dialog
    std::string command = "osascript -e 'POSIX path of (choose file with prompt \"" + 
                          title + "\" of type {\"public.data\"} default location (path to home folder))'";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    char buffer[512];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
    
#elif defined(PLATFORM_LINUX)
    // Try zenity first (GNOME)
    if (system("which zenity > /dev/null 2>&1") == 0) {
        std::string command = "zenity --file-selection --title=\"" + title + "\" 2>/dev/null";
        
        FILE* pipe = popen(command.c_str(), "r");
        if (pipe) {
            char buffer[512];
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            pclose(pipe);
            
            // Remove trailing newline
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
            
            if (!result.empty()) {
                return result;
            }
        }
    }
    
    // Try kdialog (KDE)
    if (system("which kdialog > /dev/null 2>&1") == 0) {
        std::string command = "kdialog --getopenfilename ~ '*.tap|Tape Files' --title '" + title + "' 2>/dev/null";
        
        FILE* pipe = popen(command.c_str(), "r");
        if (pipe) {
            char buffer[512];
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            pclose(pipe);
            
            // Remove trailing newline
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
            
            if (!result.empty()) {
                return result;
            }
        }
    }
    
    // No GUI available, return empty
    return "";
    
#else
    // Platform not supported
    return "";
#endif
}
