/**
 * サイクルナビゲーション - Hello World
 *
 * 最も基本的な実装から始める
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "battery/battery_manager.h"
#include "display/display_manager.h"

// 定数定義
#define SerialMon Serial
#define BATTERY_CHECK_INTERVAL 60000 // バッテリー残量チェック間隔（ミリ秒）
// #define BATTERY_CHECK_INTERVAL 5000 // バッテリー残量チェック間隔（ミリ秒）

// 使用するフォントを定義
#include "firasans_12.h"

// バッテリー管理クラスのインスタンス
BatteryManager batteryManager;

// ディスプレイ管理クラスのインスタンス
DisplayManager displayManager;

void setup()
{
  // シリアル通信の初期化
  SerialMon.begin(115200);
  delay(1000); // 安定化のための遅延

  // Hello Worldメッセージの表示（シリアル）
  SerialMon.println("Hello World!");
  SerialMon.println("サイクルナビゲーション - 初期テスト");

  // バッテリー関連の初期化（内部でI2Cの初期化も行う）
  bool battery_initialized = batteryManager.init(Wire, BOARD_SDA, BOARD_SCL);

  if (battery_initialized)
  {
    SerialMon.println("バッテリー管理システム初期化成功");
  }
  else
  {
    SerialMon.println("バッテリー管理システム初期化失敗");
  }

  // ディスプレイの初期化
  bool display_initialized = displayManager.init();

  if (display_initialized)
  {
    SerialMon.println("ディスプレイ初期化成功");
  }
  else
  {
    SerialMon.println("ディスプレイ初期化失敗");
  }

  // 回転後のディスプレイサイズを表示
  SerialMon.printf("ディスプレイサイズ: 幅 %d, 高さ %d\n",
                   displayManager.getWidth(),
                   displayManager.getHeight());

  // 画面をクリア
  displayManager.clearScreen();

  // "Hello World"テキストの描画
  displayManager.displayCenteredText("Hello World", &FiraSans_12);

  // バッテリー残量の表示
  displayManager.displayBatteryStatus(batteryManager, &FiraSans_12);

  // 画面の更新
  displayManager.updateScreen();

  SerialMon.println("E-Paperディスプレイに「Hello World」を表示しました");
}

void loop()
{
  // 60秒ごとにバッテリー残量を更新して表示
  static unsigned long lastBatteryUpdate = 0;
  if (millis() - lastBatteryUpdate > BATTERY_CHECK_INTERVAL)
  {
    lastBatteryUpdate = millis();

    // 画面をクリア
    displayManager.clearScreen();

    // "Hello World"テキストの描画
    displayManager.displayCenteredText("Hello World", &FiraSans_12);

    // バッテリー残量の表示
    displayManager.displayBatteryStatus(batteryManager, &FiraSans_12);

    // 画面の更新
    displayManager.updateScreen();

    SerialMon.printf("バッテリー残量: %d%%, 電圧: %dmV\n",
                     batteryManager.getPercent(),
                     batteryManager.getVoltage());
  }

  delay(1000);
}
