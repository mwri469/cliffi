#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>

namespace {
inline std::string trim(const std::string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return {};
    auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
}

inline std::unordered_map<std::string, std::string> load_env(const std::string& path = ".env") {
    std::unordered_map<std::string, std::string> env;
    std::ifstream file(path);
    if (!file.is_open()) file.open("../" + path);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') ||
                                 (val.front() == '\'' && val.back() == '\'')))
            val = val.substr(1, val.size() - 2);
        env[key] = val;
    }
    return env;
}
