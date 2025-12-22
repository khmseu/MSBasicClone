/**
 * @file filesystem.cpp
 * @brief Cross-platform filesystem operations for BASIC file I/O
 *
 * This file implements platform-independent filesystem operations used by
 * the BASIC interpreter for commands like LOAD, SAVE, CATALOG, and file
 * selector dialogs.
 *
 * Key features:
 * - Cross-platform file listing (Windows and POSIX)
 * - BASIC program file reading/writing with line number preservation
 * - Binary array data I/O for RECALL/STORE commands
 * - Native file selector dialogs (zenity/kdialog/osascript/Windows COM)
 * - ProDOS-compatible error messages
 *
 * Platform-specific implementations:
 * - Windows: Uses WIN32 API (FindFirstFile, CoCreateInstance for dialogs)
 * - Linux: Uses dirent.h (opendir/readdir) and zenity/kdialog
 * - macOS: Uses dirent.h and osascript for file selection
 */

#include "filesystem.h"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define chdir _chdir
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

/**
 * @brief List files in a directory
 *
 * Returns information about all files in the specified directory, excluding
 * "." and ".." entries. Uses platform-specific APIs for optimal performance.
 *
 * @param path Directory path to list
 * @return Vector of FileInfo structures with name, size, and isDirectory
 */
std::vector<FileInfo> listFiles(const std::string &path) {
  std::vector<FileInfo> files;

#ifdef PLATFORM_WINDOWS
  WIN32_FIND_DATAA findData;
  HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);

  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (strcmp(findData.cFileName, ".") != 0 &&
          strcmp(findData.cFileName, "..") != 0) {
        FileInfo info;
        info.name = findData.cFileName;
        info.size = findData.nFileSizeLow;
        info.isDirectory =
            (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        files.push_back(info);
      }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
  }
#else
  DIR *dir = opendir(path.c_str());
  if (dir) {
    struct dirent *entry;
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

/**
 * @brief Check if a file exists
 *
 * Simple existence check by attempting to open the file for reading.
 *
 * @param filename Path to file
 * @return true if file exists and is readable, false otherwise
 */
bool fileExists(const std::string &filename) {
  std::ifstream file(filename);
  return file.good();
}

/**
 * @brief Read entire text file into a string
 *
 * Reads the file contents into memory as a string. Used for loading
 * BASIC program files.
 *
 * @param filename Path to file
 * @return File contents as string
 * @throws std::runtime_error with "PATH NOT FOUND ERROR" if file cannot be
 * opened
 */
std::string readTextFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file) {
    throw std::runtime_error("PATH NOT FOUND ERROR");
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

/**
 * @brief Write an entire text file from a string
 *
 * Used by BASIC commands that create or overwrite text files, such as SAVE
 * and simple text persistence used by some ProDOS-style commands.
 *
 * Error behavior:
 * - Throws "I/O ERROR" when the file cannot be opened for writing.
 * - Overwrites existing content.
 *
 * @param filename Path to file
 * @param content Full file content to write
 * @throws std::runtime_error with "I/O ERROR" if the file cannot be written
 */
void writeTextFile(const std::string &filename, const std::string &content) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("I/O ERROR");
  }

  file << content;
}

/**
 * @brief Delete a filesystem entry
 *
 * Implements the ProDOS-style DELETE behavior used by the interpreter.
 * Returns a boolean rather than throwing so callers can map failures to
 * ProDOS-like error messages.
 *
 * @param filename Path to file
 * @return true if deletion succeeded, false otherwise
 */
bool deleteFile(const std::string &filename) {
  try {
    return fs::remove(filename);
  } catch (...) {
    return false;
  }
}

/**
 * @brief Rename a filesystem entry
 *
 * Implements the ProDOS-style RENAME behavior used by the interpreter.
 * Returns a boolean rather than throwing so callers can map failures to
 * ProDOS-like error messages.
 *
 * @param oldName Existing path
 * @param newName New path
 * @return true if rename succeeded, false otherwise
 */
bool renameFile(const std::string &oldName, const std::string &newName) {
  try {
    fs::rename(oldName, newName);
    return true;
  } catch (...) {
    return false;
  }
}

/**
 * @brief Get the current ProDOS prefix (working directory)
 *
 * The interpreter treats the process working directory as the ProDOS PREFIX.
 * This function returns that directory for commands like PREFIX with no
 * argument.
 *
 * @return Current working directory path, or empty string on failure
 */
std::string getCurrentPrefix() {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != nullptr) {
    return std::string(cwd);
  }
  return "";
}

/**
 * @brief Set the current ProDOS prefix (working directory)
 *
 * In ProDOS, PREFIX changes the default directory used to resolve partial
 * pathnames. In this host implementation, we map PREFIX to process working
 * directory.
 *
 * @param path New working directory path
 * @return true if the directory was changed successfully, false otherwise
 */
bool setPrefix(const std::string &path) { return chdir(path.c_str()) == 0; }

// FileManager implementation

/**
 * @brief Get the FileManager singleton instance
 *
 * Returns the global FileManager instance used to manage open files. Uses
 * function-local static initialization to ensure thread-safe construction.
 *
 * @return Reference to FileManager singleton
 */
FileManager &FileManager::getInstance() {
  static FileManager instance;
  return instance;
}

/**
 * @brief Open a file and return an integer handle
 *
 * Provides ProDOS-like OPEN semantics via a stable integer handle. If the
 * file is already open, the existing handle is returned. Mode mapping:
 * - READ: open for binary input
 * - WRITE: create/truncate for binary output
 * - APPEND: open or create for binary append
 *
 * Error behavior:
 * - Throws "PATH NOT FOUND ERROR" if the file cannot be opened.
 *
 * @param filename Path to file
 * @param mode Desired access mode (READ, WRITE, APPEND)
 * @return Positive integer handle
 * @throws std::runtime_error if the file cannot be opened
 */
int FileManager::openFile(const std::string &filename, FileAccessMode mode) {
  // Check if file is already open
  auto it = filenameToHandle_.find(filename);
  if (it != filenameToHandle_.end()) {
    return it->second; // Return existing handle
  }

  int handle = nextHandle_++;
  FileHandle &fh = handles_[handle];
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
    throw std::runtime_error("PATH NOT FOUND ERROR");
  }
}

/**
 * @brief Close an open file by handle
 *
 * Safe to call with an unknown handle: no action is taken.
 *
 * @param handle Handle returned by openFile()
 */
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

/**
 * @brief Close an open file by filename
 *
 * Convenience wrapper over handle-based close.
 *
 * @param filename Path used when opening the file
 */
void FileManager::closeFile(const std::string &filename) {
  auto it = filenameToHandle_.find(filename);
  if (it != filenameToHandle_.end()) {
    closeFile(it->second);
  }
}

/**
 * @brief Close all open files
 *
 * Used by interpreter lifecycle operations like NEW/CLEAR to ensure file
 * buffers are released.
 */
void FileManager::closeAllFiles() {
  for (auto &pair : handles_) {
    if (pair.second.stream && pair.second.stream->is_open()) {
      pair.second.stream->close();
    }
  }
  handles_.clear();
  filenameToHandle_.clear();
}

/**
 * @brief Lookup internal handle state
 *
 * Internal helper that returns a pointer to the FileHandle structure, or
 * nullptr if the handle is not valid.
 *
 * @param handle File handle
 * @return Pointer to FileHandle, or nullptr
 */
FileHandle *FileManager::getHandle(int handle) {
  auto it = handles_.find(handle);
  if (it != handles_.end()) {
    return &it->second;
  }
  return nullptr;
}

/**
 * @brief Read a line of text from an open file
 *
 * Implements the core behavior used by ProDOS READ/INPUT-from-file flows.
 * The function returns false on EOF.
 *
 * Error behavior:
 * - Throws "FILE NOT OPEN" if the handle is invalid.
 *
 * @param handle File handle
 * @param line Output line buffer
 * @return true if a line was read, false if EOF reached
 * @throws std::runtime_error if the file is not open
 */
bool FileManager::readLine(int handle, std::string &line) {
  FileHandle *fh = getHandle(handle);
  if (!fh || !fh->isOpen || !fh->stream) {
    throw std::runtime_error("FILE NOT OPEN");
  }

  if (std::getline(*fh->stream, line)) {
    fh->position = fh->stream->tellg();
    return true;
  }
  return false;
}

/**
 * @brief Write a line of text to an open file
 *
 * Writes the provided line followed by a newline. Intended for simple
 * sequential text output.
 *
 * Error behavior:
 * - Throws "FILE NOT OPEN" if the handle is invalid.
 *
 * @param handle File handle
 * @param line Text to write (newline appended)
 * @return true if write succeeded, false otherwise
 * @throws std::runtime_error if the file is not open
 */
bool FileManager::writeLine(int handle, const std::string &line) {
  FileHandle *fh = getHandle(handle);
  if (!fh || !fh->isOpen || !fh->stream) {
    throw std::runtime_error("FILE NOT OPEN");
  }

  *fh->stream << line << '\n';
  fh->position = fh->stream->tellp();
  return fh->stream->good();
}

/**
 * @brief Set the current byte position in an open file
 *
 * This is a low-level primitive used by future/random-access style commands.
 * For READ mode this calls seekg(); for WRITE/APPEND it calls seekp().
 *
 * @param handle File handle
 * @param position Byte offset from beginning of file
 * @throws std::runtime_error if the file is not open
 */
void FileManager::setPosition(int handle, size_t position) {
  FileHandle *fh = getHandle(handle);
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

/**
 * @brief Get the cached byte position of an open file
 *
 * Returns the internal position tracking value, which is updated after
 * successful reads/writes and after setPosition().
 *
 * @param handle File handle
 * @return Current byte offset
 * @throws std::runtime_error if the file is not open
 */
size_t FileManager::getPosition(int handle) {
  FileHandle *fh = getHandle(handle);
  if (!fh || !fh->isOpen) {
    throw std::runtime_error("FILE NOT OPEN");
  }
  return fh->position;
}

/**
 * @brief Flush buffered writes to disk
 *
 * Safe to call for read-only files; flush() is a no-op in that case.
 *
 * @param handle File handle
 */
void FileManager::flushFile(int handle) {
  FileHandle *fh = getHandle(handle);
  if (fh && fh->isOpen && fh->stream) {
    fh->stream->flush();
  }
}

/**
 * @brief Flush buffered writes by filename
 *
 * Convenience wrapper that flushes the file if it is currently open.
 *
 * @param filename Path used when opening the file
 */
void FileManager::flushFile(const std::string &filename) {
  auto it = filenameToHandle_.find(filename);
  if (it != filenameToHandle_.end()) {
    flushFile(it->second);
  }
}

/**
 * @brief Lock a file (stub)
 *
 * ProDOS supports file locking. Host-platform locking semantics are
 * platform-specific and are not yet fully implemented.
 *
 * Current behavior:
 * - Returns true if the file exists.
 * - Does not change filesystem permissions.
 *
 * @param filename Path to file
 * @return true if file exists (stub), false otherwise
 */
bool FileManager::lockFile(const std::string &filename) {
  // On Unix systems, file locking is complex and platform-specific
  // For now, we'll return true as a stub
  return fileExists(filename);
}

/**
 * @brief Unlock a file (stub)
 *
 * See lockFile() for rationale. Currently this only checks existence.
 *
 * @param filename Path to file
 * @return true if file exists (stub), false otherwise
 */
bool FileManager::unlockFile(const std::string &filename) {
  // Stub implementation
  return fileExists(filename);
}

/**
 * @brief Create an empty file
 *
 * Creates/truncates a file by opening it for output.
 *
 * @param filename Path to create
 * @return true on success, false on failure
 */
bool FileManager::createFile(const std::string &filename) {
  try {
    std::ofstream file(filename);
    return file.good();
  } catch (...) {
    return false;
  }
}

/**
 * @brief Load a file as binary bytes
 *
 * Reads the full file contents into a byte vector.
 *
 * Note: The optional address parameter is accepted for API compatibility with
 * Applesoft BLOAD semantics, but this host implementation currently treats it
 * as informational (the bytes are returned to the caller without being copied
 * into an emulated 64K memory image).
 *
 * @param filename Path to binary file
 * @param address Optional load address (unused)
 * @return File content as bytes
 * @throws std::runtime_error with "PATH NOT FOUND ERROR" if open fails
 * @throws std::runtime_error with "I/O ERROR" if read fails
 */
std::vector<uint8_t> FileManager::loadBinaryFile(const std::string &filename,
                                                 int address) {
  (void)address; // address is informational only in this host implementation
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("PATH NOT FOUND ERROR");
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    throw std::runtime_error("I/O ERROR");
  }

  return buffer;
}

/**
 * @brief Save bytes to a binary file
 *
 * Writes the provided data to a file. The address and length arguments exist
 * to match BSAVE-style calling conventions.
 *
 * Current behavior:
 * - Writes up to length bytes if length > 0, otherwise writes all data.
 * - Does not write a ProDOS binary header (address/length). Higher-level
 *   callers may add headers if desired.
 *
 * @param filename Output path
 * @param data Bytes to write
 * @param address Intended address (unused)
 * @param length Intended length; if >0, caps bytes written
 * @throws std::runtime_error with "I/O ERROR" on open/write failure
 */
void FileManager::saveBinaryFile(const std::string &filename,
                                 const std::vector<uint8_t> &data, int address,
                                 int length) {
  (void)address; // preserved for API compatibility
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    throw std::runtime_error("I/O ERROR");
  }

  size_t writeLen = (length > 0)
                        ? std::min(static_cast<size_t>(length), data.size())
                        : data.size();
  if (!file.write(reinterpret_cast<const char *>(data.data()), writeLen)) {
    throw std::runtime_error("I/O ERROR");
  }
}
