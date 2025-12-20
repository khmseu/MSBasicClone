#include "filesystem.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <filesystem>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

std::vector<FileInfo> listFiles(const std::string& path) {
    std::vector<FileInfo> files;
    
#ifdef PLATFORM_WINDOWS
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                FileInfo info;
                info.name = findData.cFileName;
                info.size = findData.nFileSizeLow;
                info.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                files.push_back(info);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(path.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                FileInfo info;
                info.name = entry->d_name;
                info.isDirectory = entry->d_type == DT_DIR;
                
                std::string fullPath = path + "/" + entry->d_name;
                struct stat st;
                if (stat(fullPath.c_str(), &st) == 0) {
                    info.size = st.st_size;
                } else {
                    info.size = 0;
                }
                
                files.push_back(info);
            }
        }
        closedir(dir);
    }
#endif
    
    return files;
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string readTextFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("FILE NOT FOUND ERROR");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeTextFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("I/O ERROR");
    }
    
    file << content;
}

bool deleteFile(const std::string& filename) {
    try {
        return fs::remove(filename);
    } catch (...) {
        return false;
    }
}

bool renameFile(const std::string& oldName, const std::string& newName) {
    try {
        fs::rename(oldName, newName);
        return true;
    } catch (...) {
        return false;
    }
}

std::string getCurrentPrefix() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        return std::string(cwd);
    }
    return "";
}

bool setPrefix(const std::string& path) {
    return chdir(path.c_str()) == 0;
}

// FileManager implementation
FileManager& FileManager::getInstance() {
    static FileManager instance;
    return instance;
}

int FileManager::openFile(const std::string& filename, FileAccessMode mode) {
    // Check if file is already open
    auto it = filenameToHandle_.find(filename);
    if (it != filenameToHandle_.end()) {
        return it->second;  // Return existing handle
    }
    
    int handle = nextHandle_++;
    FileHandle& fh = handles_[handle];
    fh.filename = filename;
    fh.mode = mode;
    fh.position = 0;
    fh.stream = std::make_unique<std::fstream>();
    
    std::ios_base::openmode openMode = std::ios_base::binary;
    switch (mode) {
        case FileAccessMode::READ:
            openMode |= std::ios_base::in;
            break;
        case FileAccessMode::WRITE:
            openMode |= std::ios_base::out | std::ios_base::trunc;
            break;
        case FileAccessMode::APPEND:
            openMode |= std::ios_base::out | std::ios_base::app;
            break;
    }
    
    fh.stream->open(filename, openMode);
    fh.isOpen = fh.stream->is_open();
    
    if (fh.isOpen) {
        filenameToHandle_[filename] = handle;
        return handle;
    } else {
        handles_.erase(handle);
        throw std::runtime_error("FILE NOT FOUND ERROR");
    }
}

void FileManager::closeFile(int handle) {
    auto it = handles_.find(handle);
    if (it != handles_.end()) {
        if (it->second.stream && it->second.stream->is_open()) {
            it->second.stream->close();
        }
        filenameToHandle_.erase(it->second.filename);
        handles_.erase(it);
    }
}

void FileManager::closeFile(const std::string& filename) {
    auto it = filenameToHandle_.find(filename);
    if (it != filenameToHandle_.end()) {
        closeFile(it->second);
    }
}

void FileManager::closeAllFiles() {
    for (auto& pair : handles_) {
        if (pair.second.stream && pair.second.stream->is_open()) {
            pair.second.stream->close();
        }
    }
    handles_.clear();
    filenameToHandle_.clear();
}

FileHandle* FileManager::getHandle(int handle) {
    auto it = handles_.find(handle);
    if (it != handles_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool FileManager::readLine(int handle, std::string& line) {
    FileHandle* fh = getHandle(handle);
    if (!fh || !fh->isOpen || !fh->stream) {
        throw std::runtime_error("FILE NOT OPEN");
    }
    
    if (std::getline(*fh->stream, line)) {
        fh->position = fh->stream->tellg();
        return true;
    }
    return false;
}

bool FileManager::writeLine(int handle, const std::string& line) {
    FileHandle* fh = getHandle(handle);
    if (!fh || !fh->isOpen || !fh->stream) {
        throw std::runtime_error("FILE NOT OPEN");
    }
    
    *fh->stream << line << '\n';
    fh->position = fh->stream->tellp();
    return fh->stream->good();
}

void FileManager::setPosition(int handle, size_t position) {
    FileHandle* fh = getHandle(handle);
    if (!fh || !fh->isOpen || !fh->stream) {
        throw std::runtime_error("FILE NOT OPEN");
    }
    
    if (fh->mode == FileAccessMode::READ) {
        fh->stream->seekg(position);
    } else {
        fh->stream->seekp(position);
    }
    fh->position = position;
}

size_t FileManager::getPosition(int handle) {
    FileHandle* fh = getHandle(handle);
    if (!fh || !fh->isOpen) {
        throw std::runtime_error("FILE NOT OPEN");
    }
    return fh->position;
}

void FileManager::flushFile(int handle) {
    FileHandle* fh = getHandle(handle);
    if (fh && fh->isOpen && fh->stream) {
        fh->stream->flush();
    }
}

void FileManager::flushFile(const std::string& filename) {
    auto it = filenameToHandle_.find(filename);
    if (it != filenameToHandle_.end()) {
        flushFile(it->second);
    }
}

bool FileManager::lockFile(const std::string& filename) {
    // On Unix systems, file locking is complex and platform-specific
    // For now, we'll return true as a stub
    return fileExists(filename);
}

bool FileManager::unlockFile(const std::string& filename) {
    // Stub implementation
    return fileExists(filename);
}

bool FileManager::createFile(const std::string& filename) {
    try {
        std::ofstream file(filename);
        return file.good();
    } catch (...) {
        return false;
    }
}

std::vector<uint8_t> FileManager::loadBinaryFile(const std::string& filename, int address) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("FILE NOT FOUND ERROR");
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("I/O ERROR");
    }
    
    return buffer;
}

void FileManager::saveBinaryFile(const std::string& filename, const std::vector<uint8_t>& data, int address, int length) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("I/O ERROR");
    }
    
    size_t writeLen = (length > 0) ? std::min(static_cast<size_t>(length), data.size()) : data.size();
    if (!file.write(reinterpret_cast<const char*>(data.data()), writeLen)) {
        throw std::runtime_error("I/O ERROR");
    }
}
