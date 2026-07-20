/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "tickers/list.hpp"
#include "commands/handler.hpp"
#include "search/search.hpp"
#include "api/lse.hpp"
#include "graph/chart.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

int 
main(void) 
{
    using namespace ftxui;

    std::string search_str;
    std::string cmd_str;
    std::string time_str;
    std::string status_msg;
    std::atomic<bool> running     = true;
    bool         show_graph       = false;
    std::string  graph_ticker;
    std::atomic<bool> graph_loading = false;
    std::mutex   graph_prices_mutex;
    std::vector<double> graph_prices;
    std::atomic<int>    graph_gen = 0; // incremented per request; old threads self-cancel

    auto screen    = ScreenInteractive::Fullscreen();
    auto securities = std::make_shared<SecurityList>();

    // Resolve asset paths — try CWD (project root) then parent (if running from build/)
    std::string symbols_path = "assets/symbols.txt";
    {
        std::ifstream test(symbols_path);
        if (!test.is_open()) symbols_path = "../assets/symbols.txt";
    }

    // When the user selects a ticker: fetch intraday data asynchronously
    securities->set_on_select([&](const std::string& ticker) {
        int my_gen = ++graph_gen;
        graph_ticker  = ticker;
        show_graph    = true;
        graph_loading = true;
        {
            std::lock_guard<std::mutex> lock(graph_prices_mutex);
            graph_prices.clear();
        }
        std::thread([&, ticker, my_gen] {
            auto prices = LSE::fetch_intraday(ticker);
            if (my_gen != graph_gen) return; // superseded by a newer request
            {
                std::lock_guard<std::mutex> lock(graph_prices_mutex);
                graph_prices = prices;
            }
            graph_loading = false;
            if (running)
                if (auto* s = ScreenInteractive::Active()) s->PostEvent(Event::Custom);
        }).detach();
    });

    // Clock thread — updates time_str every second
    auto time_thread = std::thread([&] {
        while (running) {
            auto now        = std::chrono::system_clock::now();
            auto in_time_t  = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buffer{};
            localtime_r(&in_time_t, &tm_buffer);
            std::ostringstream ss;
            ss << std::put_time(&tm_buffer, "%H:%M:%S");
            time_str = ss.str();
            if (auto* s = ScreenInteractive::Active()) s->PostEvent(Event::Custom);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    // Search input — Enter searches symbols file and adds first match
    InputOption search_opt;
    search_opt.on_enter = [&] {
        auto first = search_str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            search_str.clear();
            return;
        }
        auto last = search_str.find_last_not_of(" \t\r\n");
        search_str = search_str.substr(first, last - first + 1);
        if (search_str.empty()) return;
        if (search_str[0] == '/') {
            execute_command(search_str, *securities, running, screen);
            search_str.clear();
            return;
        }
        auto results = search_symbols(search_str, symbols_path);
        if (!results.empty()) {
            // Prefer an exact match if one exists
            std::string upper = search_str;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            std::string match = results[0];
            for (const auto& r : results)
                if (r == upper) { match = r; break; }
            securities->add_ticker(match);
            status_msg = "Added: " + match;
        } else {
            std::ifstream verify(symbols_path);
            if (!verify.is_open())
                status_msg = "Symbols file not found: " + symbols_path;
            else
                status_msg = "Not found: " + search_str;
        }
        search_str.clear();
    };
    Component input_search = Input(&search_str, "ticker...", search_opt);

    // Cmd input — Enter dispatches command (/quit, /save, ...)
    InputOption cmd_opt;
    cmd_opt.on_enter = [&] {
        execute_command(cmd_str, *securities, running, screen);
        cmd_str.clear();
    };
    Component input_cmd = Input(&cmd_str, "/cmd", cmd_opt);

    Component securities_component = securities->create_component();

    auto component = Container::Vertical({
        input_search,
        input_cmd,
        securities_component,
    });

    auto renderer = Renderer(component, [&] {
        Element context_content;

        if (show_graph) {
            Element graph_el;
            if (graph_loading) {
                graph_el = text("Fetching " + graph_ticker + "...") | center | flex;
            } else {
                std::vector<double> prices_copy;
                {
                    std::lock_guard<std::mutex> lock(graph_prices_mutex);
                    prices_copy = graph_prices;
                }
                graph_el = render_chart(prices_copy, graph_ticker) | flex;
            }
            context_content = hbox({
                securities_component->Render() | flex,
                separator(),
                graph_el,
            });
        } else {
            context_content = securities_component->Render();
        }

        return vbox({
            hbox({
                window(text("Search"), input_search->Render()) | flex,
                window(text("Cmd"),    input_cmd->Render())    | size(WIDTH, EQUAL, 14),
                window(text("Time"),   text(time_str))         | size(WIDTH, EQUAL, 12),
            }),
            window(text("Context"), context_content) | flex,
            text(status_msg) | dim,
        });
    });

    // Escape dismisses the graph view
    auto root = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Escape && show_graph) {
            show_graph = false;
            return true;
        }
        return false;
    });

    screen.Loop(root);

    // Cleanup
    running = false;
    securities->is_running() = false;
    if (time_thread.joinable())
        time_thread.join();

    return 0;
}