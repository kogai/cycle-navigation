#pragma once

#include <Arduino.h>
#include <XPowersLib.h>
#include "bq27220.h"
#include "config.h"

class PowerManager
{
public:
  PowerManager();

  /**
   * @brief 初期化処理
   * @return 初期化成功でtrue、失敗でfalse
   */
  bool init();

  /**
   * @brief バッテリー残量を取得
   * @return バッテリー残量（%）
   */
  int getBatteryLevel();

  /**
   * @brief バッテリー電圧を取得
   * @return バッテリー電圧（mV）
   */
  int getBatteryVoltage();

  /**
   * @brief 充電状態を取得
   * @return 充電中ならtrue、そうでなければfalse
   */
  bool isCharging();

  /**
   * @brief 低バッテリー状態かどうかを確認
   * @return 低バッテリーならtrue、そうでなければfalse
   */
  bool isLowBattery();

  /**
   * @brief 緊急バッテリー状態かどうかを確認
   * @return 緊急バッテリーならtrue、そうでなければfalse
   */
  bool isCriticalBattery();

  /**
   * @brief ディープスリープモードに入る
   * @param time_us スリープ時間（マイクロ秒）
   */
  void enterDeepSleep(uint64_t time_us = DEEP_SLEEP_DURATION);

  /**
   * @brief GPSモジュールの電源をオン
   */
  void enableGPS();

  /**
   * @brief GPSモジュールの電源をオフ
   */
  void disableGPS();

  /**
   * @brief LoRaモジュールの電源をオン
   */
  void enableLoRa();

  /**
   * @brief LoRaモジュールの電源をオフ
   */
  void disableLoRa();

  /**
   * @brief 定期的な電源管理タスク（バッテリー監視など）
   */
  void update();

private:
  XPowersPPM PPM;
  BQ27220 bq27220;
  unsigned long lastBatteryCheck;
  int batteryLevel;
  int batteryVoltage;
  bool charging;

  /**
   * @brief バッテリー情報を更新
   */
  void updateBatteryInfo();
};

extern PowerManager powerManager;
