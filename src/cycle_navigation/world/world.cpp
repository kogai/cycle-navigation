/**
 * 描画サイクル管理モジュール実装
 */

#include "world.h"

// コンストラクタ
World::World(Battery &battery, Display &display)
    : battery(battery), display(display), lastBatteryUpdate(0), initialized(false)
{
}
