/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include <chrono>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip> 

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/util/ref.hpp"

class SecurityList
{
// Functions
private:
    void _load_tickers();
public:
    SecurityList(/* args */);
    ~SecurityList();

// Attributes
private:
    std::string     _ticker_path = "/../assets/tickers.txt"
    std::vector     _tickers;
public:
};
