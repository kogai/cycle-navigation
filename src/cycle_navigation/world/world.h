/**
 * 描画サイクル管理モジュール
 *
 * バッテリーとディスプレイの状態を管理し、描画サイクルを制御する
 */

#pragma once

#include <Arduino.h>
#include "../battery/battery.h"
#include "../display/display.h"
#include "../config.h"

class World
{
public:
  World(Battery &battery, Display &display);
  void init(TwoWire &wire, int sda, int scl);
  void render();

private:
  void render_();
  Battery &battery;
  Display &display;
};
