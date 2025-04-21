/**
* Copyright 2024 Martin Wright. All rights reserved.
* Use of this source code is governed by the license found in the LICENSE file.
*/
#include <chrono>
#include <memory>
#include <string>
#include <iomanip> 
#include <sstream>
#include "ftxui/util/ref.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"

int 
main(void) 
{
    using namespace ftxui;

    std::string search;
    std::string cmd;
    std::string time_str;
    std::atomic<bool> running = true;
    Component ui;

    auto time_thread = std::thread([&] {
        while (running) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buffer;
        localtime_r(&in_time_t, &tm_buffer);

        std::stringstream ss;
        ss << std::put_time(&tm_buffer, "%H:%M:%S");
        time_str = ss.str();

        // Request redraw
        ScreenInteractive::Active()->PostEvent(Event::Custom);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });       

    Component input_search = Input(&search, "");
    Component input_cmd = Input(&cmd, "/");

    auto component = Container::Vertical({
        input_search,
        input_cmd,
    });

    auto renderer = Renderer(component, [&] {    
        return vbox({
            hbox({
                window(text("Search"), input_search->Render())  | flex,
                window(text("Cmd"), input_cmd->Render())        | size(WIDTH, EQUAL, 10),
                window(text("Time"), text(time_str))            | size(WIDTH, EQUAL, 10),
            }), window(text("UI"), text(""))                    | flex,
            });
    });

    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(renderer);

    // Cleanup
    running = false;
    if (time_thread.joinable()) {
    time_thread.join();
    }

    return 0;
}
