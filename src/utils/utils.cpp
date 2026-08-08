#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <Windows.h>

#include "utils.h"

namespace fs = std::filesystem;
fs::path getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return fs::path(buffer);
}

/*
    Load GLSL shaders from a given file.
    FilePath: src/shaders/test.txt
*/
std::string loadShaderFrom(const std::string& filePath) {
    fs::path executablePath = getExecutablePath();
    fs::path dataPath = executablePath.parent_path().parent_path() / filePath;
    if (!fs::exists(dataPath))
        throw std::runtime_error("Couldn't find shader path.");

    std::ifstream file(dataPath);
    if (!file.is_open())
        throw std::runtime_error("Shader couldn't be opened.");
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}