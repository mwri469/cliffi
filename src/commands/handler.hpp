/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#pragma once

#include <atomic>
#include <string>

#include "ftxui/component/screen_interactive.hpp"
#include "tickers/list.hpp"

void execute_command(const std::string& cmd, SecurityList& securities,
                     std::atomic<bool>& running, ftxui::ScreenInteractive& screen);
