/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#pragma once

#include <string>
#include <vector>

// Returns tickers from symbols_path that contain query (case-insensitive).
// Exact matches are sorted to the front.
std::vector<std::string> search_symbols(const std::string& query,
                                         const std::string& symbols_path);
