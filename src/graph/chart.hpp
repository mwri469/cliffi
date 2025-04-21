/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#pragma once

#include <string>
#include <vector>

#include "ftxui/dom/elements.hpp"

// Render an FTXUI line chart for intraday price history.
ftxui::Element render_chart(const std::vector<double>& prices,
                             const std::string& ticker);
