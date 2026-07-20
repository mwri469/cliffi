/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include "search.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

std::vector<std::string> 
search_symbols(const std::string& query, const std::string& symbols_path)
{
    if (query.empty()) return {};

    std::string upper_query = query;
    std::transform(upper_query.begin(), upper_query.end(), upper_query.begin(), ::toupper);

    std::ifstream file(symbols_path);
    if (!file.is_open()) file.open("../" + symbols_path);
    std::vector<std::string> exact, partial;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::string upper_line = line;
        std::transform(upper_line.begin(), upper_line.end(), upper_line.begin(), ::toupper);

        if (upper_line == upper_query)
            exact.push_back(upper_line);
        else if (upper_line.find(upper_query) != std::string::npos)
            partial.push_back(upper_line);
    }

    exact.insert(exact.end(), partial.begin(), partial.end());
    return exact;
}
