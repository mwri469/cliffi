#pragma once

#include <string>
#include <vector>

#include "tickers/list.hpp"

namespace LSE {
    // Fetch current quote data for a ticker symbol via LSE /candles endpoint.
    SecurityData fetch_quote(const std::string& ticker);

    // Fetch intraday close prices (5-minute interval, today's session).
    // Returns an empty vector on failure.
    std::vector<double> fetch_intraday(const std::string& ticker);
}
