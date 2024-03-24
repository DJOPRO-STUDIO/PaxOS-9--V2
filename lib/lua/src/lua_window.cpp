#include "lua_window.hpp"

#include <iostream>

LuaWindow::LuaWindow()  
{
    widget = new gui::elements::Window();
    init(widget);
}

int LuaWindow::delete_LuaWindow(LuaWindow* window) {
    std::cout << "delete_LuaWindow" << std::endl;
    delete window;
    return 0;
}