#include "lse.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "utils/env.hpp"

namespace {

const std::string BASE_URL = "https://api.londonstrategicedge.com/vault";

std::string get_api_key() {
    static std::string key;
    if (key.empty()) {
        auto env = load_env();
        key = env["LSE_API_KEY"];
    }
    return key;
}

size_t write_cb(void* ptr, size_t size, size_t nmemb, std::string* out) {
    out->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::string fetch_url(const std::string& url) {
    std::string api_key = get_api_key();
    if (api_key.empty()) return {};

    CURL* curl = curl_easy_init();
    if (!curl) return {};

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, ("x-api-key: " + api_key).c_str());

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

std::string today_date() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");
    return ss.str();
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

// Extract a double value for a given JSON key from a single JSON object.
// Handles format: "key": 123.45
double parse_json_number(const std::string& json, const std::string& key, size_t start = 0) {
    std::string needle = "\"" + key + "\":";
    auto pos = json.find(needle, start);
    if (pos == std::string::npos) return 0.0;
    pos += needle.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size() || json[pos] == 'n') return 0.0;
    try {
        return std::stod(json.substr(pos));
    } catch (...) {
        return 0.0;
    }
}

// Find the start of each object in a JSON array.
// Returns positions of each '{' that starts a top-level array element.
std::vector<size_t> find_objects(const std::string& json) {
    std::vector<size_t> positions;
    // Find the opening '['
    auto arr_start = json.find('[');
    if (arr_start == std::string::npos) return positions;

    int depth = 0;
    for (size_t i = arr_start + 1; i < json.size(); i++) {
        if (json[i] == '{') {
            if (depth == 0) positions.push_back(i);
            depth++;
        } else if (json[i] == '}') {
            depth--;
        } else if (json[i] == ']' && depth == 0) {
            break;
        }
    }
    return positions;
}

} // namespace

namespace LSE {

SecurityData fetch_quote(const std::string& ticker) {
    // Get last 2 daily candles to compute change from previous close
    std::string url = BASE_URL + "/candles?symbol=" + ticker
                    + "&timeframe=1d&limit=2&order=desc";
    auto json = fetch_url(url);
    if (json.empty()) return {ticker};

    auto objects = find_objects(json);
    if (objects.empty()) return {ticker};

    // First object is the most recent day
    double price = parse_json_number(json, "close", objects[0]);
    double prev  = objects.size() >= 2
                 ? parse_json_number(json, "close", objects[1])
                 : price;

    double change     = price - prev;
    double change_pct = prev > 0.0 ? (change / prev * 100.0) : 0.0;

    return {ticker, price, change, change_pct, current_time_str()};
}

std::vector<double> fetch_intraday(const std::string& ticker) {
    std::string url = BASE_URL + "/candles?symbol=" + ticker
                    + "&timeframe=5m&limit=78&order=desc";
    auto json = fetch_url(url);
    if (json.empty()) return {};

    auto objects = find_objects(json);
    std::vector<double> closes;
    closes.reserve(objects.size());
    for (auto pos : objects) {
        double c = parse_json_number(json, "close", pos);
        if (c > 0.0) closes.push_back(c);
    }
    std::reverse(closes.begin(), closes.end());
    return closes;
}

} // namespace LSE
