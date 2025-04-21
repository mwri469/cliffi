/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include "yahoo.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <curl/curl.h>

namespace {

size_t write_cb(void* ptr, size_t size, size_t nmemb, std::string* out) {
    out->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::string fetch_url(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return {};

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers,
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36");

    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,1L);

    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

// Extract the first occurrence of a numeric JSON field by key name.
double parse_json_number(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return 0.0;
    pos += needle.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size() || json[pos] == 'n') return 0.0; // null
    try {
        return std::stod(json.substr(pos));
    } catch (...) {
        return 0.0;
    }
}

// Extract the numeric array immediately following "close":[
std::vector<double> parse_close_array(const std::string& json) {
    auto pos = json.find("\"close\":[");
    if (pos == std::string::npos) return {};
    pos += 9;
    auto end = json.find(']', pos);
    if (end == std::string::npos) return {};

    std::vector<double> result;
    std::istringstream ss(json.substr(pos, end - pos));
    std::string token;
    while (std::getline(ss, token, ',')) {
        auto s = token.find_first_not_of(" \t\n\r");
        if (s == std::string::npos) continue;
        token = token.substr(s);
        if (token == "null") continue;
        try {
            result.push_back(std::stod(token));
        } catch (...) {}
    }
    return result;
}

std::string current_time_str() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    return ss.str();
}

} // namespace

namespace Yahoo {

SecurityData fetch_quote(const std::string& ticker) {
    std::string url = "https://query2.finance.yahoo.com/v8/finance/chart/"
                    + ticker + "?interval=1d&range=1d";
    auto json = fetch_url(url);
    if (json.empty()) return {ticker};

    double price     = parse_json_number(json, "regularMarketPrice");
    double prev      = parse_json_number(json, "chartPreviousClose");
    double change    = price - prev;
    double change_pct = prev > 0.0 ? (change / prev * 100.0) : 0.0;

    return {ticker, price, change, change_pct, current_time_str()};
}

std::vector<double> fetch_intraday(const std::string& ticker) {
    std::string url = "https://query2.finance.yahoo.com/v8/finance/chart/"
                    + ticker + "?interval=5m&range=1d";
    auto json = fetch_url(url);
    if (json.empty()) return {};
    return parse_close_array(json);
}

} // namespace Yahoo
