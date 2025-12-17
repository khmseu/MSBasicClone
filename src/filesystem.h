#pragma once

// File system operations for DOS/ProDOS compatibility
#include <string>
#include <vector>

struct FileInfo {
    std::string name;
    size_t size;
    bool isDirectory;
};

std::vector<FileInfo> listFiles(const std::string& path = ".");
bool fileExists(const std::string& filename);
std::string readTextFile(const std::string& filename);
void writeTextFile(const std::string& filename, const std::string& content);
