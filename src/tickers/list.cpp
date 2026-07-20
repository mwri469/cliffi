/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "list.hpp"
#include "api/lse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/table.hpp"
#include "ftxui/util/ref.hpp"

namespace {

class SecuritiesComponent : public ftxui::ComponentBase {
public:
    explicit SecuritiesComponent(SecurityList* list) : _list(list) {}

    bool Focusable() const override { return true; }

    ftxui::Element Render() override {
        using namespace ftxui;
        return window(text("Securities"),
            hbox({_list->render_table(Focused()), filler() | flex}));
    }

    bool OnEvent(ftxui::Event event) override {
        using namespace ftxui;
        if (event == Event::ArrowDown) { _list->move_selection(1);  return true; }
        if (event == Event::ArrowUp)   { _list->move_selection(-1); return true; }
        if (event == Event::Return)    { _list->trigger_select();    return true; }
        return false;
    }

private:
    SecurityList* _list;
};

} // namespace

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
            _update_prices();
            ftxui::ScreenInteractive::Active()->PostEvent(ftxui::Event::Custom);
            std::this_thread::sleep_for(std::chrono::seconds(15));
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
    std::ifstream file(_ticker_path);
    if (!file.is_open()) file.open("../" + _ticker_path);
    if (!file.is_open()) {
        _tickers = {"AAPL", "MSFT", "GOOG", "AMZN", "META"};
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty())
            _tickers.push_back(line);
    }
}

void 
SecurityList::_update_prices() 
{
    std::vector<std::string> tickers_copy;
    {
        std::lock_guard<std::mutex> lock(_data_mutex);
        tickers_copy = _tickers;
    }

    for (const auto& ticker : tickers_copy) {
        if (!_is_running) return;
        auto quote = LSE::fetch_quote(ticker);
        if (quote.price <= 0.0) continue;
        std::lock_guard<std::mutex> lock(_data_mutex);
        _securities_data[ticker] = quote;
    }
}

std::vector<std::vector<std::string>>
SecurityList::_generate_table_data() 
{
    // Caller must hold _data_mutex
    std::vector<std::vector<std::string>> table_data;
    table_data.push_back({"Ticker", "Price", "Chg", "Chg%", "Last Update"});
    
    for (const auto& ticker : _tickers) {
        const auto& data = _securities_data.at(ticker);
        std::stringstream price_ss, change_ss, pct_ss;
        price_ss  << std::fixed << std::setprecision(2) << data.price;
        change_ss << std::fixed << std::setprecision(2) << data.change;
        pct_ss    << std::fixed << std::setprecision(2) << data.change_percent << "%";
        table_data.push_back({
            data.ticker, price_ss.str(), change_ss.str(), pct_ss.str(), data.last_update
        });
    }
    
    return table_data;
}

void 
SecurityList::update_data() 
{
    _update_prices();
}

ftxui::Element 
SecurityList::render_table(bool focused) 
{
    using namespace ftxui;
    
    std::lock_guard<std::mutex> lock(_data_mutex);
    auto table_data = _generate_table_data();
    auto table = Table(table_data);
    
    table.SelectAll().Border(LIGHT);
    table.SelectRow(0).Decorate(bold);
    table.SelectRow(0).Border(DOUBLE);
    table.SelectColumn(1).DecorateCells(align_right);
    table.SelectColumn(2).DecorateCells(align_right);
    table.SelectColumn(3).DecorateCells(align_right);

    // Selection highlight (applied before change colors so colors take precedence)
    int sel_row = _selected_index + 1;
    if (sel_row > 0 && sel_row < (int)table_data.size()) {
        if (focused)
            table.SelectRow(sel_row).Decorate(inverted);
        else
            table.SelectRow(sel_row).Decorate(bold);
    }
    
    for (int i = 1; i < (int)table_data.size(); i++) {
        const auto& data = _securities_data.at(table_data[i][0]);
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
    return ftxui::Make<SecuritiesComponent>(this);
}

void 
SecurityList::add_ticker(const std::string& ticker) 
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    for (const auto& t : _tickers)
        if (t == ticker) return;
    _tickers.push_back(ticker);
    _securities_data[ticker] = {ticker, 100.0, 0.0, 0.0, "--:--:--"};
}

void 
SecurityList::remove_ticker(const std::string& ticker) 
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    _tickers.erase(std::remove(_tickers.begin(), _tickers.end(), ticker), _tickers.end());
    _securities_data.erase(ticker);
    if (_selected_index >= (int)_tickers.size())
        _selected_index = std::max(0, (int)_tickers.size() - 1);
}

void 
SecurityList::move_selection(int delta) 
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    if (_tickers.empty()) return;
    _selected_index = std::clamp(_selected_index + delta, 0, (int)_tickers.size() - 1);
}

void 
SecurityList::trigger_select() 
{
    std::function<void(const std::string&)> cb;
    std::string ticker;
    {
        std::lock_guard<std::mutex> lock(_data_mutex);
        if (_tickers.empty() || !_on_select) return;
        cb = _on_select;
        ticker = _tickers[_selected_index];
    }
    cb(ticker);
}

std::string 
SecurityList::selected_ticker() const 
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    if (_tickers.empty()) return "";
    return _tickers[_selected_index];
}

void 
SecurityList::set_on_select(std::function<void(const std::string&)> cb) 
{
    _on_select = std::move(cb);
}

void 
SecurityList::save_tickers() const 
{
    std::ofstream file(_ticker_path);
    if (!file.is_open()) file.open("../" + _ticker_path);
    std::lock_guard<std::mutex> lock(_data_mutex);
    for (const auto& ticker : _tickers)
        file << ticker << '\n';
}