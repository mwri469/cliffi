/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include "handler.hpp"

#include <algorithm>
#include <cctype>
#include <string>

void 
execute_command(const std::string& raw_cmd, SecurityList& securities,
                std::atomic<bool>& running, ftxui::ScreenInteractive& screen)
{
    auto start = raw_cmd.find_first_not_of(" \t");
    if (start == std::string::npos) return;
    std::string cmd = raw_cmd.substr(start);
    auto end = cmd.find_last_not_of(" \t\r\n");
    if (end == std::string::npos) return;
    cmd.resize(end + 1);

    // Strip leading slash if present
    if (!cmd.empty() && cmd[0] == '/') cmd = cmd.substr(1);

    // Normalise to lowercase for comparison
    std::string lower = cmd;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "quit") {
        securities.save_tickers();
        running = false;
        securities.is_running() = false;
        screen.Exit();
    } else if (lower == "save") {
        securities.save_tickers();
    }
}
