//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "graphics.hpp"

#include <iostream>
#include <Surface.hpp>
#include <threads.hpp>

#ifdef ESP_PLATFORM

#include "platforms/LGFX_ESP32_PAXO5.hpp"

#else

#include "lgfx/v1/platforms/sdl/Panel_sdl.hpp"
#include "LGFX_AUTODETECT.hpp"

#include "LovyanGFX.hpp"

#endif

namespace
{
    bool running;

    std::shared_ptr<LGFX> lcd;

    graphics::EScreenOrientation screenOrientation;
    std::shared_ptr<graphics::Surface> landscapeBuffer;
}

void graphics::setBrightness(uint16_t value)
{
    lcd->setBrightness(value);
}

void graphics::init()
{
#ifdef ESP_PLATFORM

    running = true; // It doesn't feel right to set this here...

    lcd = std::make_shared<LGFX>();

#else

    lcd = std::make_shared<LGFX>(getScreenWidth(), getScreenHeight());

    // We need to create a "landscape buffer" used as a screen.
    // Because LovyanGFX as a weird color glitch when using "setRotation()".
    // But, by using a temporary buffer, the glitch doesn't appear.
    landscapeBuffer = std::make_shared<Surface>(getScreenHeight(), getScreenWidth());

#endif

    lcd->init();
    lcd->setColorDepth(16);
    lcd->setTextColor(TFT_WHITE);
    lcd->fillScreen(TFT_RED);
}

void graphics::reInit()
{
    lcd->init();
    lcd->setTextColor(TFT_WHITE);
    lcd->fillScreen(TFT_RED);
}

uint16_t graphics::getScreenWidth()
{
    switch (screenOrientation)
    {
    case graphics::PORTRAIT:
        return 320;
    case graphics::LANDSCAPE:
        return 480;
    }

    return -1;
}

uint16_t graphics::getScreenHeight()
{
    switch (screenOrientation)
    {
    case graphics::PORTRAIT:
        return 480;
    case graphics::LANDSCAPE:
        return 320;
    }

    return -1;
}

bool graphics::isRunning()
{
    return running;
}

#ifndef ESP_PLATFORM

struct SDLUpdateData
{
    void (*appMain)();
};

static int SDLUpdate(void *data)
{
    const auto *updateData = static_cast<SDLUpdateData *>(data);

    updateData->appMain();

    return 0;
}

void graphics::SDLInit(void (*appMain)())
{
    lgfx::Panel_sdl::setup();
    // lgfx::Panel_sdl::loop(); // Ensure to create the window before creating a new thread

    SDLUpdateData updateData
    {
        appMain
    };

    running = true;

    // Multithreading can be an issue, be careful
    SDL_Thread *thread = SDL_CreateThread(SDLUpdate, "graphics_update", &updateData);
    if (thread == nullptr)
    {
        printf("Unable to create thread : %s\n", SDL_GetError());
        exit(1);
    }

    while (lgfx::Panel_sdl::loop() == 0)
    {};

    running = false;

    // SDL_WaitThread(thread, nullptr);

    lgfx::Panel_sdl::close();
}

#endif

// You should only use this function with a "Canvas" (Surface that is the size of the screen)
void graphics::showSurface(const Surface* surface, int x, int y)
{
    lgfx::LGFX_Sprite sprite = surface->m_sprite; // we are friends !

#ifdef ESP_PLATFORM
    sprite.pushSprite(lcd.get(), x, y);
#else
    if (screenOrientation == LANDSCAPE)
    {
        landscapeBuffer->pushSurface(const_cast<Surface *>(surface), static_cast<int16_t>(x), static_cast<int16_t>(y));
        landscapeBuffer->m_sprite.pushSprite(lcd.get(), 0, 0);
    }
    else
    {
        sprite.pushSprite(lcd.get(), x, y);
    }
#endif
}

void graphics::flip()
{
    // lcd->display();
}

void graphics::getTouchPos(int16_t* x, int16_t* y)
{
    static uint64_t lastUpdate = millis();
    static int16_t otx = 0;
    static int16_t oty = 0;

    if(lastUpdate + 50 > millis())
    {
        *x = otx;
        *y = oty;

        return;
    }

    int16_t tx;
    int16_t ty;

    //std::cout << "touch -> " << std::endl;
    lcd->getTouch(&tx, &ty);
    //std::cout << " -> Ok" << std::endl;
    //std::cout << "state: " << lcd->getTouch(&tx, &ty) << " ty: " << ty * 480 / 700 << "connected: " << int(ty<-1 || ty> 480 * 480 / 700) << std::endl;
    #ifdef ESP_PLATFORM // with capacitive touch?
        if(screenOrientation == 0)
            ty = ty * 480 / 700;
        else
            tx = (float) (tx - 50) * 5.5 / 7.6; // * 27 / 37 fait augmenter le nombre, pourquoi??
    #endif

    if (tx <= 0 || ty <= 0 || tx > graphics::getScreenWidth() || ty > graphics::getScreenHeight())
    {
        // Be sure to be offscreen
        otx = -1;
        oty = -1;
    }
    else
    {
        otx = tx;
        oty = ty;
    }

    *x = otx;
    *y = oty;
}

bool graphics::isTouched()
{
    int16_t x;
    int16_t y;

    getTouchPos(&x, &y);

    return x != -1 && y != -1;
}

graphics::EScreenOrientation graphics::getScreenOrientation()
{
    return screenOrientation;
}

void graphics::setScreenOrientation(const EScreenOrientation screenOrientation)
{
    // Update the screen orientation (and the screen size)
    // Maybe use another name for the parameter ?
    // Or store it in another place ?
    ::screenOrientation = screenOrientation;

    switch (screenOrientation)
    {
    case PORTRAIT:
        lcd->setRotation(0);
        break;
    case LANDSCAPE:
        lcd->setRotation(1);
        break;
    }
}
