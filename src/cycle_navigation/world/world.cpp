/**
 * 描画サイクル管理モジュール実装
 */

#include "world.h"
#include "firasans_12.h"

World::World(Battery &battery, Display &display)
    : battery(battery), display(display)
{
}

void World::init(TwoWire &wire, int sda, int scl)
{
    battery.init(wire, sda, scl);
    display.init();
    // display.clearScreen();
}

void World::render()
{
    display.clearScreen();
    // epd_poweron();
    // epd_clear();
    display.displayCenteredText("Hello World", &FiraSans_12);
    display.displayBatteryStatus(battery, &FiraSans_12);
    // epd_poweroff();
    display.updateScreen();
    // epd_write_string(font, batteryText, &cursor_x, &cursor_y, fb, &font_props);
}
