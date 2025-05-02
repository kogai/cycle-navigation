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
  // コンストラクタ
  World(Battery &battery, Display &display);

  // 描画処理
  void render();

private:
  // バッテリーとディスプレイへの参照
  Battery &battery;
  Display &display;
};
