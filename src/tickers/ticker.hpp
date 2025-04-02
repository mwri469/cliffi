/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include <vector>
#include <memory>
#include <sstream>
#include <string>
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/util/ref.hpp"
#include "ticker.hpp"

class ticker
////////////////////////////////////////////////
// PROPERTIES //////////////////////////////////
////////////////////////////////////////////////
{
private:
    /* Data */
public:
    std::string ticker;
    vector<float>       opens;
    vector<float>       closes;
    vector<float>       highs;
    vector<float>       lows;
    int         interval;
    std::string exchange;
    std::string currency;

////////////////////////////////////////////////
// FUNCTIONS ///////////////////////////////////
////////////////////////////////////////////////
public:
    ticker(/* args */);
    ~ticker();

    void        open_list(void);
    
};
