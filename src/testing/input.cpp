// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <memory>  // for allocator, __shared_ptr_access
#include <string>  // for char_traits, operator+, string, basic_string
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref
 
int main() {
  using namespace ftxui;
 
  // The data:
  std::string search;
  std::string cmd;
 
  // The basic input components:
  Component input_search = Input(&search, "");
  Component input_cmd = Input(&cmd, "/");
 
  // The component tree:
  auto component = Container::Vertical({
      input_search,
      input_cmd,
  });
 
  // Tweak how the component tree is rendered:
  auto renderer = Renderer(component, [&] {
    return vbox({
               hbox({
                window(text("Search"), input_search->Render())  | flex, 
                window(text("Cmd"), input_cmd->Render())        | size(WIDTH, EQUAL, 20),
                window(text("Time"), text("17:36"))             | size(WIDTH, EQUAL, 20),
                }),
           });
  });
 
  auto screen = ScreenInteractive::TerminalOutput();
  screen.Loop(renderer);
}
