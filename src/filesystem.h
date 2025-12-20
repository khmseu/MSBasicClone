#pragma once

// File system operations for DOS/ProDOS compatibility
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <memory>

struct FileInfo {
    std::string name;
    size_t size;
    bool isDirectory;
};

// File access modes
enum class FileAccessMode {
    READ,
    WRITE,
    APPEND
};

// File handle structure
struct FileHandle {
    std::string filename;
    FileAccessMode mode;
    std::unique_ptr<std::fstream> stream;
    size_t position;
    bool isOpen;
    
    FileHandle() : position(0), isOpen(false) {}
};

// File system operations
std::vector<FileInfo> listFiles(const std::string& path = ".");
bool fileExists(const std::string& filename);
std::string readTextFile(const std::string& filename);
void writeTextFile(const std::string& filename, const std::string& content);

// ProDOS-compatible file operations
bool deleteFile(const std::string& filename);
bool renameFile(const std::string& oldName, const std::string& newName);
std::string getCurrentPrefix();
bool setPrefix(const std::string& path);

// File handle management
class FileManager {
public:
    static FileManager& getInstance();
    
    // Open/close files
    int openFile(const std::string& filename, FileAccessMode mode);
    void closeFile(int handle);
    void closeFile(const std::string& filename);
    void closeAllFiles();
    
    // File I/O
    bool readLine(int handle, std::string& line);
    bool writeLine(int handle, const std::string& line);
    void setPosition(int handle, size_t position);
    size_t getPosition(int handle);
    
    // Flush operations
    void flushFile(int handle);
    void flushFile(const std::string& filename);
    
    // File operations
    bool lockFile(const std::string& filename);
    bool unlockFile(const std::string& filename);
    bool createFile(const std::string& filename);
    
    // Binary file operations
    std::vector<uint8_t> loadBinaryFile(const std::string& filename, int address = -1);
    void saveBinaryFile(const std::string& filename, const std::vector<uint8_t>& data, int address, int length);
    
private:
    FileManager() : nextHandle_(1) {}
    std::map<int, FileHandle> handles_;
    std::map<std::string, int> filenameToHandle_;
    int nextHandle_;
    
    FileHandle* getHandle(int handle);
};
