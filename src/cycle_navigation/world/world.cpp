/**
 * 描画サイクル管理モジュール実装
 */

#include "world.h"

World::World(Battery &battery, Display &display)
    : battery(battery), display(display)
{
}

void World::init(TwoWire &wire, int sda, int scl)
{
    battery.init(wire, sda, scl);
    display.init();
}
