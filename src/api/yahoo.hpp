/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#pragma once

#include <string>
#include <vector>

#include "tickers/list.hpp"

namespace Yahoo {
    // Fetch current quote data for a ticker symbol.
    SecurityData fetch_quote(const std::string& ticker);

    // Fetch intraday close prices (5-minute interval, 1-day range).
    // Returns an empty vector on failure.
    std::vector<double> fetch_intraday(const std::string& ticker);
}
