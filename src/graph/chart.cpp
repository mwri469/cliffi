/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include "chart.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

ftxui::Element 
render_chart(const std::vector<double>& prices, const std::string& ticker) 
{
    using namespace ftxui;

    if (prices.empty())
        return text("No data for " + ticker) | center;

    double min_p = *std::min_element(prices.begin(), prices.end());
    double max_p = *std::max_element(prices.begin(), prices.end());
    double range = (max_p - min_p < 0.001) ? 1.0 : (max_p - min_p);

    // Capture by value so the lambda is safe across threads
    auto graph_fn = [prices, min_p, range](int width, int height) -> std::vector<int> {
        std::vector<int> out(width, 0);
        int n = (int)prices.size();
        for (int x = 0; x < width; ++x) {
            int idx = x * n / width;
            if (idx < n)
                out[x] = (int)((prices[idx] - min_p) / range * (height - 1));
        }
        return out;
    };

    std::ostringstream lo, hi, last;
    lo   << std::fixed << std::setprecision(2) << min_p;
    hi   << std::fixed << std::setprecision(2) << max_p;
    last << std::fixed << std::setprecision(2) << prices.back();

    double chg     = prices.back() - prices.front();
    double chg_pct = prices.front() > 0.0 ? (chg / prices.front() * 100.0) : 0.0;
    std::ostringstream chg_ss;
    chg_ss << std::showpos << std::fixed << std::setprecision(2)
           << chg << "  " << chg_pct << "%";

    auto chg_color = chg >= 0.0 ? Color::Green : Color::Red;

    return vbox({
        hbox({
            text(ticker) | bold,
            text("  "),
            text(last.str()) | bold,
            text("  "),
            text(chg_ss.str()) | color(chg_color),
        }) | center,
        graph(graph_fn) | color(Color::Green) | flex,
        hbox({
            text(lo.str()) | dim,
            filler() | flex,
            text(hi.str()) | dim,
        }),
        text("  5m  |  1d") | dim | center,
    });
}
