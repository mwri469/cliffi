/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib> 
#include <sstream>
#include <iomanip> 

#include "list.hpp"
#include "ftxui/util/ref.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"

SecurityList::SecurityList(/* args */)
{
}

SecurityList::~SecurityList()
{
}

void 
SecurityList::_load_tickers(void) 
{
    // First, get data path:
    std::string src_dir = std::getenv("CMAKE_SRC_DIR");
    std::string ticker_file = src_dir + _ticker_path;

    // Open the file
    std::ifstream file(ticker_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << ticker_file << std::endl;
        return;
    }

    // Read each line and store it in the vector
    std::string line;
    while (std::getline(file, line)) {
        _tickers.push_back(line);
    }

    // Close the file
    file.close();

    // Print the tickers to verify
    for (const auto& ticker : _tickers) {
        std::cout << ticker << std::endl;
    }
}

