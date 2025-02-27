#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <esp_system.h>

#endif

#include "graphics.hpp"
#include "hardware.hpp"
#include "gui.hpp"
#include "path.hpp"
#include "filestream.hpp"
#include "threads.hpp"
#include "lua_file.hpp"
#include "gsm.hpp"
#include "app.hpp"
#include <iostream>

using namespace gui::elements;

void loop(){}

void ringingVibrator()
{
    #ifdef ESP_PLATFORM
    if(GSM::state.callState == GSM::CallState::RINGING)
    {
        hardware::setVibrator(true); delay(100); hardware::setVibrator(false);
    }
    #endif
}


void setup()
{    
    hardware::init();
    hardware::setScreenPower(true);
    graphics::init();
    storage::init();

    graphics::setScreenOrientation(graphics::PORTRAIT);

    ThreadManager::init();

    GSM::ExternalEvents::onIncommingCall = []()
    {
        app::App call;
        call.auth = true;
        call.name = "phone";
        call.manifest = storage::Path("/sys_apps/root.json");
        call.path = storage::Path("/sys_apps/call.lua");

        app::requestingApp = call;
        app::request = true;
    };

    #ifdef ESP_PLATFORM
    eventHandlerBack.setInterval(
        new Callback<>(&ringingVibrator),
        300
    );
    #endif

    app::init();

    while (true)
    {
        #ifdef ESP_PLATFORM
        graphics::setBrightness(0xFF/3);
        #endif

        int l = 0;
        while (l!=-1)
        {
            l = launcher();
            if(l!=-1)
                app::runApp(app::appList[l].path);

            while (hardware::getHomeButton());
        }

        graphics::setBrightness(0);
        StandbyMode::savePower();


        while (hardware::getHomeButton());
        while (!hardware::getHomeButton())
        {
            eventHandlerApp.update();
        }
        
        StandbyMode::restorePower();
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(setup);
}

#endif
