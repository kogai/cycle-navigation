/**
 * バッテリー管理モジュール
 *
 * バッテリーの状態監視と管理を行う
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "bq27220.h"
#define XPOWERS_CHIP_BQ25896
#include <XPowersLib.h>
#include "../config.h"

// BQ25896のI2Cアドレス
#define BQ25896_SLAVE_ADDRESS 0x6B

class BatteryManager
{
public:
  // コンストラクタ
  BatteryManager();

  // 初期化
  bool init(TwoWire &wire, int sda, int scl);

  // バッテリー残量を取得
  uint16_t getPercent();

  // バッテリー電圧を取得
  uint16_t getVoltage();

  // バッテリーが初期化されているか確認
  bool isInitialized() const;

private:
  // バッテリー関連のオブジェクト
  BQ27220 bq27220;
  XPowersPPM PPM;

  // 初期化フラグ
  bool initialized;
};
