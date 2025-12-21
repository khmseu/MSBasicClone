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

TapeManager::~TapeManager() {
    close();
}

void TapeManager::setTapeFile(const std::string& filename) {
    if (isOpen()) {
        close();
    }
    currentTape_ = filename;
    position_ = 0;
}

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

void TapeManager::close() {
    if (file_.is_open()) {
        file_.close();
    }
    position_ = 0;
}

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

bool TapeManager::isEndOfTape() const {
    if (!isOpen() || !readMode_) {
        return false;
    }
    return file_.eof();
}

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
