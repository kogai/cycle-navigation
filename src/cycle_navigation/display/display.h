/**
 * ディスプレイ管理モジュール
 *
 * E-Paperディスプレイの初期化と描画を管理する
 */

#pragma once

#include <Arduino.h>
#include <epdiy.h>
#include "../config.h"
#include "../battery/battery.h"

class Display
{
public:
  // コンストラクタ
  Display();

  // 初期化
  bool init();

  // 画面をクリア
  void clearScreen();

  // テキストを中央に表示
  void displayCenteredText(const char *text, const EpdFont *font);

  // バッテリー情報を表示
  void displayBatteryStatus(Battery &battery, const EpdFont *font);

  // 画面を更新
  void updateScreen();

  // フレームバッファを取得
  uint8_t *getFramebuffer();

  // 回転後のディスプレイ幅を取得
  int getWidth();

  // 回転後のディスプレイ高さを取得
  int getHeight();

private:
  // 使用するボードとディスプレイの設定
  // 実装ファイルで具体的な値を設定

  // ハイレベル状態の変数
  EpdiyHighlevelState hl;

  // 初期化フラグ
  bool initialized;
};
