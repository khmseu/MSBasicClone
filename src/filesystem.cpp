#include "filesystem.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
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
