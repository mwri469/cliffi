/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/table.hpp"
#include "ftxui/util/ref.hpp"

struct SecurityData {
    std::string ticker;
    double price = 0.0;
    double change = 0.0;
    double change_percent = 0.0;
    std::string last_update;
};

class SecurityList {
// Functions
private:
    void _load_tickers();
    void _update_mock_prices(); // For mock data
    std::vector<std::vector<std::string>> _generate_table_data();

public:
    SecurityList();
    ~SecurityList();
    
    void update_data();
    ftxui::Element render_table();
    ftxui::Component create_component();

// Attributes
private:
    std::string _ticker_path = "/../assets/tickers.txt";
    std::vector<std::string> _tickers;
    std::map<std::string, SecurityData> _securities_data;
    std::atomic<bool> _is_running = true;
    std::thread _update_thread;

public:
    std::atomic<bool>& is_running() { return _is_running; }
};