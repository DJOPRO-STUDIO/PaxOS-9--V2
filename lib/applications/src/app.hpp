#ifndef APPS_HPP
#define APPS_HPP

#include <vector>
#include <path.hpp>
#include <filestream.hpp>
#include <lua_file.hpp>
#include <gui.hpp>
#include <hardware.hpp>
#include <../network/network.hpp>

#define APP_DIR "/apps"
#define PERMS_DIR "/system"


namespace app
{
    struct App
    {
        std::string name;
        storage::Path path;
        storage::Path manifest;
        bool auth;
    };

    extern std::vector<App> appList;

    extern bool request;
    extern App requestingApp;

    void init();
    bool askPerm(App &app);
    void runApp(storage::Path path);
    void runApp(App app);
};

#include <launcher.hpp>

#endif