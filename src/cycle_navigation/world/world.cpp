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
}

void World::render()
{
    // TODO: PWRボタン押下イベントへのコールバック登録
    // TODO: タイマーイベントへのコールバック登録
    // TODO: ディープスリープ
}

void World::render_()
{
    // display.clearScreen();
    // display.displayCenteredText("Hello World?", &FiraSans_12);
    // // display.displayBatteryStatus(battery, &FiraSans_12);
    // display.updateScreen();
}
