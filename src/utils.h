#pragma once
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <filesystem>

inline std::filesystem::path resolvePath(const std::string& relative) {
    namespace fs = std::filesystem;
    fs::path relPath(relative);
    fs::path base = fs::current_path();
    for (int i = 0; i < 6; ++i) {
        if (!base.empty()) {
            fs::path candidate = base / relPath;
            std::error_code ec;
            if (fs::exists(candidate, ec)) {
                fs::path canonicalPath = fs::canonical(candidate, ec);
                return ec ? candidate : canonicalPath;
            }
            if (!base.has_parent_path()) {
                break;
            }
            base = base.parent_path();
        }
    }
    return relPath;
}

inline std::vector<char> readFile(const std::string& filename) {
    const std::filesystem::path resolvedPath = resolvePath(filename);
    std::ifstream file(resolvedPath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + resolvedPath.string());
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}