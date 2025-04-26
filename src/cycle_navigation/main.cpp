/**
 * サイクルナビゲーション - Hello World
 *
 * 最も基本的な実装から始める
 */

#include <Arduino.h>
#include "config.h"

void setup()
{
  // シリアル通信の初期化
  SerialMon.begin(115200);
  delay(1000); // 安定化のための遅延

  // Hello Worldメッセージの表示
  SerialMon.println("Hello World!");
  SerialMon.println("サイクルナビゲーション - 初期テスト");
}

void loop()
{
  // 1秒ごとにメッセージを表示
  SerialMon.println("動作中...");
  delay(1000);
}
