#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

// Tape Manager for emulating cassette tape operations
// Provides sequential file access with record headers
class TapeManager {
public:
    TapeManager();
    ~TapeManager();

    // Tape file management
    void setTapeFile(const std::string& filename);
    std::string getTapeFile() const { return currentTape_; }
    bool hasTape() const { return !currentTape_.empty(); }

    // Tape operations
    void openForRead();
    void openForWrite();
    void close();
    void rewind();
    
    // Record I/O with size headers (SIZE|DATA format)
    void writeRecord(const std::vector<uint8_t>& data);
    std::vector<uint8_t> readRecord();
    
    // Status
    bool isOpen() const { return file_.is_open(); }
    bool isReadMode() const { return readMode_; }
    bool isWriteMode() const { return !readMode_; }
    size_t getPosition() const { return position_; }
    bool isEndOfTape() const;

    // File selector (OS-specific)
    static std::string showFileSelector(const std::string& title = "Select Tape File");

private:
    std::string currentTape_;
    std::fstream file_;
    bool readMode_;
    size_t position_;
    
    void updatePosition();
};
