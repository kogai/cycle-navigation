/**
 * サイクルナビゲーション - Hello World
 *
 * 最も基本的な実装から始める
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "battery/battery.h"
#include "display/display.h"
#include "world/world.h"

// 定数定義
#define SerialMon Serial
#define BATTERY_CHECK_INTERVAL 60000 // バッテリー残量チェック間隔（ミリ秒）
// #define BATTERY_CHECK_INTERVAL 5000 // バッテリー残量チェック間隔（ミリ秒）

Battery battery;
Display display;
World world(battery, display);

void setup()
{
  // シリアル通信の初期化 TODO: Debugモードのみ
  SerialMon.begin(115200);

  world.init(Wire, BOARD_SDA, BOARD_SCL);
  world.start();
  SerialMon.println("サイクルナビゲーション - 初期テスト");
}

void loop()
{
}
