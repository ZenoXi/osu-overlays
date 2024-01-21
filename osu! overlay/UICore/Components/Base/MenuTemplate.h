#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <optional>

namespace zcom
{
    // TODO:
    // In future possible to make template menu item contain a factory that creates menu items using lambdas
    // This would allow the menu to contain arbitrary components as long as they adhere to some interface
    // Whether or not this is needed is unclear, since the menu usually is used for simple controls, and
    // implementing this, while maybe not terribly complicated, right now is very much not a priority
    //

    struct MenuTemplate
    {
        struct MenuItem;

        struct Menu
        {
            Menu() {}

            std::vector<MenuTemplate::MenuItem> items;
        };

        struct MenuItem
        {
            MenuItem() {}

            std::wstring text;
            std::string iconResourceName;

            bool separator = false;
            int checkGroup = -1;
            bool checkable = false;
            bool closeOnClick = false;
            std::function<void(bool)> onClick;
            std::optional<MenuTemplate::Menu> nestedMenu;
        };

        MenuTemplate::Menu baseMenu;
    };
}