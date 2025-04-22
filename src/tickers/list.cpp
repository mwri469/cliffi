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
#include <iostream>
#include <random>
#include <thread>

#include "list.hpp"
#include "ftxui/util/ref.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/table.hpp"

SecurityList::SecurityList()
{
    _load_tickers();
    
    // Initialize securities data with default values
    for (const auto& ticker : _tickers) {
        _securities_data[ticker] = {ticker, 100.0, 0.0, 0.0, "00:00:00"};
    }
    
    // Start update thread
    _update_thread = std::thread([this] {
        while (_is_running) {
            _update_mock_prices();
            
            // Request redraw
            ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
            
            // Update every 2 seconds
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });
}

SecurityList::~SecurityList()
{
    _is_running = false;
    if (_update_thread.joinable()) {
        _update_thread.join();
    }
}

void 
SecurityList::_load_tickers(void) 
{
    // First, get data path:
    const char* src_dir_env = std::getenv("CMAKE_SRC_DIR");
    std::string src_dir = src_dir_env ? src_dir_env : ".";
    std::string ticker_file = src_dir + _ticker_path;

    // Open the file
    std::ifstream file(ticker_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << ticker_file << std::endl;
        
        // For testing, add some default tickers if file can't be opened
        _tickers = {"AAPL", "MSFT", "GOOG", "AMZN", "META"};
        return;
    }

    // Read each line and store it in the vector
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            _tickers.push_back(line);
        }
    }

    // Close the file
    file.close();

    // Print the tickers to verify
    std::cout << "Loaded tickers: ";
    for (const auto& ticker : _tickers) {
        std::cout << ticker << " ";
    }
    std::cout << std::endl;
}

void 
SecurityList::_update_mock_prices() 
{
    // Random number generation for mock price changes
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> price_change(-3.0, 3.0);
    
    // Get current time for last update
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buffer;
    localtime_r(&in_time_t, &tm_buffer);
    
    std::stringstream time_ss;
    time_ss << std::put_time(&tm_buffer, "%H:%M:%S");
    std::string current_time = time_ss.str();
    
    // Update each security's price
    for (auto& [ticker, data] : _securities_data) {
        double change = price_change(gen);
        double old_price = data.price;
        data.price += change;
        if (data.price < 1.0) data.price = 1.0;  // Prevent negative prices
        
        data.change = data.price - old_price;
        data.change_percent = (data.change / old_price) * 100.0;
        data.last_update = current_time;
    }
}

std::vector<std::vector<std::string>>
SecurityList::_generate_table_data() 
{
    std::vector<std::vector<std::string>> table_data;
    
    // Add header row
    table_data.push_back({"Ticker", "Price", "Chg", "Chg%", "Last Update"});
    
    // Add data rows
    for (const auto& ticker : _tickers) {
        if (_securities_data.find(ticker) != _securities_data.end()) {
            const auto& data = _securities_data[ticker];
            
            std::stringstream price_ss, change_ss, change_percent_ss;
            price_ss << std::fixed << std::setprecision(2) << data.price;
            change_ss << std::fixed << std::setprecision(2) << data.change;
            change_percent_ss << std::fixed << std::setprecision(2) << data.change_percent << "%";
            
            table_data.push_back({
                data.ticker,
                price_ss.str(),
                change_ss.str(),
                change_percent_ss.str(),
                data.last_update
            });
        }
    }
    
    return table_data;
}

void 
SecurityList::update_data() 
{
    _update_mock_prices();
}

ftxui::Element 
SecurityList::render_table() 
{
    using namespace ftxui;
    
    // Generate table data
    auto table_data = _generate_table_data();
    
    // Create table
    auto table = Table(table_data);
    
    // Style the table
    table.SelectAll().Border(LIGHT);
    
    // Style header row
    table.SelectRow(0).Decorate(bold);
    table.SelectRow(0).Border(DOUBLE);
    
    // Align numbers to the right
    table.SelectColumn(1).DecorateCells(align_right);  // Price
    table.SelectColumn(2).DecorateCells(align_right);  // Change
    table.SelectColumn(3).DecorateCells(align_right);  // Change %
    
    // Color the changes (green for positive, red for negative)
    for (size_t i = 1; i < table_data.size(); i++) {
        const auto& ticker = table_data[i][0];
        const auto& data = _securities_data[ticker];
        
        if (data.change > 0) {
            table.SelectCell(i, 2).Decorate(color(Color::Green));
            table.SelectCell(i, 3).Decorate(color(Color::Green));
        } else if (data.change < 0) {
            table.SelectCell(i, 2).Decorate(color(Color::Red));
            table.SelectCell(i, 3).Decorate(color(Color::Red));
        }
    }
    
    return table.Render();
}

ftxui::Component 
SecurityList::create_component() 
{
    using namespace ftxui;
    
    return Renderer([this] {
        return 
        window(text("Securities"), 
                hbox({
                    render_table(),
                    filler() | flex,
                })
            ); // EndWindow
    }); // EndRenderer
}