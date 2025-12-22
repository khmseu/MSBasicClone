/**
 * @file filesystem.h
 * @brief DOS/ProDOS-compatible file system operations
 * 
 * Provides file I/O operations compatible with Apple II DOS and ProDOS commands:
 * - File management (CATALOG, DELETE, RENAME)
 * - Directory operations (PREFIX for working directory)
 * - Sequential file I/O (OPEN, READ, WRITE, CLOSE)
 * - Binary file loading (BLOAD/BSAVE)
 * - File locking/unlocking
 * 
 * The FileManager singleton maintains open file handles and manages file state,
 * providing ProDOS-style file operations while using standard C++ filesystem APIs.
 * 
 * Error handling:
 * - File operations return error codes matching ProDOS conventions
 * - Error codes stored in memory location 222 (accessible via PEEK)
 * - Operations throw RuntimeError for critical failures
 * 
 * File access modes:
 * - READ: Sequential reading from file
 * - WRITE: Sequential writing to file (creates/truncates)
 * - APPEND: Sequential writing starting at end of file
 * 
 * File handles:
 * - Integer handles allocated sequentially starting from 1
 * - Files can be accessed by handle or filename
 * - Multiple files can be open simultaneously
 * - Automatic cleanup on program termination
 */

#pragma once

// File system operations for DOS/ProDOS compatibility
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <memory>

/**
 * @struct FileInfo
 * @brief File or directory metadata
 * 
 * Used by CATALOG command to display directory listings.
 */
struct FileInfo {
    std::string name;   ///< File or directory name
    size_t size;        ///< Size in bytes
    bool isDirectory;   ///< true if directory, false if file
};

/**
 * @enum FileAccessMode
 * @brief File access mode for opening files
 */
enum class FileAccessMode {
    READ,    ///< Read-only access
    WRITE,   ///< Write access (truncates existing file)
    APPEND   ///< Append access (writes at end)
};

/**
 * @struct FileHandle
 * @brief Internal file handle structure
 * 
 * Maintains state for an open file including stream, position, and mode.
 */
struct FileHandle {
    std::string filename;                  ///< Full path to file
    FileAccessMode mode;                   ///< Access mode
    std::unique_ptr<std::fstream> stream;  ///< File stream
    size_t position;                       ///< Current byte position
    bool isOpen;                           ///< Open state
    
    /**
     * @brief Default constructor
     */
    FileHandle() : position(0), isOpen(false) {}
};

// ============================================================================
// File system operations (free functions)
// ============================================================================

/**
 * @brief List files in directory (CATALOG)
 * @param path Directory path (default: current directory)
 * @return Vector of FileInfo structures
 */
std::vector<FileInfo> listFiles(const std::string& path = ".");

/**
 * @brief Check if file exists
 * @param filename File path
 * @return true if file exists and is accessible
 */
bool fileExists(const std::string& filename);

/**
 * @brief Read entire text file into string
 * @param filename File path
 * @return File contents as string
 * @throws RuntimeError if file cannot be read
 */
std::string readTextFile(const std::string& filename);

/**
 * @brief Write string to text file
 * @param filename File path
 * @param content String to write
 * @throws RuntimeError if file cannot be written
 */
void writeTextFile(const std::string& filename, const std::string& content);

// ============================================================================
// ProDOS-compatible file operations
// ============================================================================

/**
 * @brief Delete file (DELETE)
 * @param filename File to delete
 * @return true if successful
 */
bool deleteFile(const std::string& filename);

/**
 * @brief Rename file (RENAME)
 * @param oldName Current filename
 * @param newName New filename
 * @return true if successful
 */
bool renameFile(const std::string& oldName, const std::string& newName);

/**
 * @brief Get current working directory prefix (PREFIX)
 * @return Current directory path
 */
std::string getCurrentPrefix();

/**
 * @brief Set current working directory prefix (PREFIX)
 * @param path New working directory
 * @return true if successful
 */
bool setPrefix(const std::string& path);

// ============================================================================
// File handle management
// ============================================================================

/**
 * @class FileManager
 * @brief Singleton managing open file handles for BASIC file I/O
 * 
 * Provides ProDOS-style file operations with integer handles. Maintains
 * open file state and coordinates file access across BASIC commands.
 * 
 * Thread safety: Not thread-safe. Single-threaded use only.
 */
class FileManager {
public:
    /**
     * @brief Get FileManager singleton instance
     * @return Reference to FileManager
     */
    static FileManager& getInstance();
    
    // Open/close files
    
    /**
     * @brief Open file and return handle (OPEN)
     * @param filename File path
     * @param mode Access mode (READ/WRITE/APPEND)
     * @return File handle (positive integer)
     * @throws RuntimeError if file cannot be opened
     */
    int openFile(const std::string& filename, FileAccessMode mode);
    
    /**
     * @brief Close file by handle (CLOSE)
     * @param handle File handle from openFile()
     */
    void closeFile(int handle);
    
    /**
     * @brief Close file by filename (CLOSE)
     * @param filename File path
     */
    void closeFile(const std::string& filename);
    
    /**
     * @brief Close all open files
     * 
     * Called on program termination or NEW command.
     */
    void closeAllFiles();
    
    // File I/O
    
    /**
     * @brief Read line from file (READ#)
     * @param handle File handle
     * @param line String to store line (output parameter)
     * @return true if line read successfully, false on EOF
     */
    bool readLine(int handle, std::string& line);
    
    /**
     * @brief Write line to file (WRITE#)
     * @param handle File handle
     * @param line String to write (newline appended)
     * @return true if successful
     */
    bool writeLine(int handle, const std::string& line);
    
    /**
     * @brief Set file position (seek)
     * @param handle File handle
     * @param position Byte offset from start of file
     */
    void setPosition(int handle, size_t position);
    
    /**
     * @brief Get current file position
     * @param handle File handle
     * @return Current byte offset
     */
    size_t getPosition(int handle);
    
    // Flush operations
    
    /**
     * @brief Flush buffered writes to disk (FLUSH#)
     * @param handle File handle
     */
    void flushFile(int handle);
    
    /**
     * @brief Flush buffered writes by filename (FLUSH)
     * @param filename File path
     */
    void flushFile(const std::string& filename);
    
    // File operations
    
    /**
     * @brief Lock file (LOCK)
     * @param filename File path
     * @return true if successful
     * 
     * ProDOS file locking simulation. Implementation is platform-specific.
     */
    bool lockFile(const std::string& filename);
    
    /**
     * @brief Unlock file (UNLOCK)
     * @param filename File path
     * @return true if successful
     */
    bool unlockFile(const std::string& filename);
    
    /**
     * @brief Create empty file (CREATE)
     * @param filename File path
     * @return true if successful
     */
    bool createFile(const std::string& filename);
    
    // Binary file operations
    
    /**
     * @brief Load binary file (BLOAD)
     * @param filename File path
     * @param address Memory address (-1 to ignore address in file)
     * @return Vector of bytes read from file
     * @throws RuntimeError on I/O error
     * 
     * Loads binary data with optional address header matching ProDOS format.
     */
    std::vector<uint8_t> loadBinaryFile(const std::string& filename, int address = -1);
    
    /**
     * @brief Save binary file (BSAVE)
     * @param filename File path
     * @param data Byte vector to write
     * @param address Starting memory address (for header)
     * @param length Number of bytes to write
     * @throws RuntimeError on I/O error
     * 
     * Saves binary data with address header matching ProDOS format.
     */
    void saveBinaryFile(const std::string& filename, const std::vector<uint8_t>& data, int address, int length);
    
private:
    /**
     * @brief Private constructor for singleton
     */
    FileManager() : nextHandle_(1) {}
    
    std::map<int, FileHandle> handles_;           ///< Map of handle to file state
    std::map<std::string, int> filenameToHandle_; ///< Map of filename to handle
    int nextHandle_;                              ///< Next available handle number
    
    /**
     * @brief Get FileHandle by handle number
     * @param handle File handle
     * @return Pointer to FileHandle or nullptr if not found
     */
    FileHandle* getHandle(int handle);
};
