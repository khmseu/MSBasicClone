/**
 * @file tape_manager.h
 * @brief Cassette tape emulation for sequential data storage
 * 
 * The TapeManager class emulates Apple II cassette tape operations, providing
 * sequential file access with record-based I/O. This mimics the behavior of
 * BASIC programs that used cassette tapes for data storage.
 * 
 * Features:
 * - Sequential read/write operations
 * - Record-based I/O with size headers (SIZE|DATA format)
 * - Rewind capability
 * - Platform-specific file selector dialog
 * - State management (read/write mode, position)
 * 
 * Record format:
 * Each record is stored as a size header followed by data bytes:
 * - 4 bytes: Record size (little-endian)
 * - N bytes: Record data
 * 
 * Usage:
 * @code
 * TapeManager tape;
 * tape.setTapeFile("data.tape");
 * tape.openForWrite();
 * tape.writeRecord({0x01, 0x02, 0x03});
 * tape.close();
 * @endcode
 * 
 * The TapeManager is used by BASIC commands that access tape storage,
 * providing a simple sequential data persistence mechanism.
 */

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

/**
 * @class TapeManager
 * @brief Cassette tape emulation for sequential data storage
 * 
 * Emulates Apple II cassette tape operations with record-based sequential I/O.
 * Each record has a size header followed by data bytes, mimicking the format
 * used by Apple II tape storage.
 * 
 * The manager maintains tape state including current position, read/write mode,
 * and provides rewind capability matching physical tape behavior.
 */
class TapeManager {
public:
    /**
     * @brief Construct tape manager (no tape loaded)
     */
    TapeManager();
    
    /**
     * @brief Destructor, closes tape if open
     */
    ~TapeManager();

    // Tape file management
    
    /**
     * @brief Set tape file path
     * @param filename Path to tape file
     * 
     * Sets the tape file but doesn't open it. Call openForRead() or
     * openForWrite() to actually open the file.
     */
    void setTapeFile(const std::string& filename);
    
    /**
     * @brief Get current tape file path
     * @return Tape file path or empty string if no tape set
     */
    std::string getTapeFile() const { return currentTape_; }
    
    /**
     * @brief Check if tape file is set
     * @return true if tape file path is configured
     */
    bool hasTape() const { return !currentTape_.empty(); }

    // Tape operations
    
    /**
     * @brief Open tape for reading
     * @throws RuntimeError if no tape set or file cannot be opened
     */
    void openForRead();
    
    /**
     * @brief Open tape for writing
     * @throws RuntimeError if no tape set or file cannot be opened
     * 
     * Opens file in append mode to preserve existing data.
     */
    void openForWrite();
    
    /**
     * @brief Close tape
     * 
     * Flushes and closes the tape file. Safe to call if not open.
     */
    void close();
    
    /**
     * @brief Rewind tape to beginning
     * @throws RuntimeError if tape not open
     */
    void rewind();
    
    // Record I/O with size headers (SIZE|DATA format)
    
    /**
     * @brief Write record to tape
     * @param data Byte vector to write as record
     * @throws RuntimeError if tape not open for writing
     * 
     * Writes size header (4 bytes) followed by data bytes.
     */
    void writeRecord(const std::vector<uint8_t>& data);
    
    /**
     * @brief Read record from tape
     * @return Byte vector containing record data
     * @throws RuntimeError if tape not open for reading or read error
     * 
     * Reads size header then data bytes. Returns empty vector at EOF.
     */
    std::vector<uint8_t> readRecord();
    
    // Status
    
    /**
     * @brief Check if tape is open
     * @return true if open for reading or writing
     */
    bool isOpen() const { return file_.is_open(); }
    
    /**
     * @brief Check if tape is open for reading
     * @return true if in read mode
     */
    bool isReadMode() const { return readMode_; }
    
    /**
     * @brief Check if tape is open for writing
     * @return true if in write mode
     */
    bool isWriteMode() const { return !readMode_; }
    
    /**
     * @brief Get current tape position
     * @return Byte offset from start of tape
     */
    size_t getPosition() const { return position_; }
    
    /**
     * @brief Check if at end of tape
     * @return true if at EOF
     */
    bool isEndOfTape() const;

    // File selector (OS-specific)
    
    /**
     * @brief Show platform-specific file selector dialog
     * @param title Dialog window title
     * @return Selected file path or empty string if cancelled
     * 
     * Uses native file dialog on supported platforms. Returns empty
     * string on platforms without GUI support.
     */
    static std::string showFileSelector(const std::string& title = "Select Tape File");

private:
    std::string currentTape_;  ///< Current tape file path
    std::fstream file_;        ///< File stream for tape I/O
    bool readMode_;            ///< true for read mode, false for write mode
    size_t position_;          ///< Current byte position in tape
    
    /**
     * @brief Update position after I/O operation
     */
    void updatePosition();
};
    void updatePosition();
};
