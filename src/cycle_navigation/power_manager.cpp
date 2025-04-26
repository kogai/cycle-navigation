#include "power_manager.h"
#include <esp_sleep.h>
#include <esp_adc_cal.h>
#include <driver/gpio.h>
#include "ExtensionIOXL9555.hpp"

// グローバルインスタンス
PowerManager powerManager;

// IO拡張チップ（クラス内で使用するためにstaticに変更）
static ExtensionIOXL9555 io;

PowerManager::PowerManager() : lastBatteryCheck(0),
                               batteryLevel(0),
                               batteryVoltage(0),
                               charging(false)
{
}

bool PowerManager::init()
{
  // BQ25896の初期化
  bool result = PPM.init(Wire, BOARD_SDA, BOARD_SCL, BQ25896_SLAVE_ADDRESS);
  if (!result)
  {
    Serial.println("Failed to initialize BQ25896");
    return false;
  }

  // BQ27220の初期化
  if (!bq27220.init())
  {
    Serial.println("Failed to initialize BQ27220");
    return false;
  }

  // IO拡張チップの初期化
  if (!io.init(Wire, BOARD_SDA, BOARD_SCL, XL9555_SLAVE_ADDRESS0))
  {
    Serial.println("Failed to initialize IO extender");
    return false;
  }

  // IO拡張ポートの設定
  io.configPort(ExtensionIOXL9555::PORT0, 0x00); // PORT0を出力に設定
  io.configPort(ExtensionIOXL9555::PORT1, 0xFF); // PORT1を入力に設定

  // 電源管理の設定
  // 最小動作電圧の設定（この電圧以下になると保護モードに入る）
  PPM.setSysPowerDownVoltage(3300);

  // 入力電流制限の設定（デフォルトは500mA）
  PPM.setInputCurrentLimit(3250);

  // 電流制限ピンを無効化
  PPM.disableCurrentLimitPin();

  // 充電目標電圧の設定（範囲: 3840〜4608mV、ステップ: 16mV）
  PPM.setChargeTargetVoltage(4208);

  // プリチャージ電流の設定（範囲: 64mA〜1024mA、ステップ: 64mA）
  PPM.setPrechargeCurr(64);

  // 充電電流の設定（範囲: 0〜5056mA、ステップ: 64mA）
  PPM.setChargerConstantCurr(1024);

  // 電圧データを取得するためにADCを有効化
  PPM.enableMeasure();

  // 充電機能をオン
  PPM.enableCharge();

  // ウェイクアップソースの設定
  esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_UP_PIN, 0);

  // 初期バッテリー情報の更新
  updateBatteryInfo();

  return true;
}

int PowerManager::getBatteryLevel()
{
  return batteryLevel;
}

int PowerManager::getBatteryVoltage()
{
  return batteryVoltage;
}

bool PowerManager::isCharging()
{
  return charging;
}

bool PowerManager::isLowBattery()
{
  return batteryLevel <= LOW_BATTERY_THRESHOLD;
}

bool PowerManager::isCriticalBattery()
{
  return batteryLevel <= CRITICAL_BATTERY_THRESHOLD;
}

void PowerManager::enterDeepSleep(uint64_t time_us)
{
  Serial.println("Entering deep sleep mode...");

  // GPSとLoRaを無効化して電力消費を抑える
  disableGPS();
  disableLoRa();

  // ディープスリープ時間の設定
  esp_sleep_enable_timer_wakeup(time_us);

  // ディープスリープモードに入る
  esp_deep_sleep_start();
}

void PowerManager::enableGPS()
{
  // IO拡張チップを使用してGPSの電源をオン
  io.digitalWrite(ExtensionIOXL9555::IO0, HIGH);
  Serial.println("GPS power enabled");
}

void PowerManager::disableGPS()
{
  // IO拡張チップを使用してGPSの電源をオフ
  io.digitalWrite(ExtensionIOXL9555::IO0, LOW);
  Serial.println("GPS power disabled");
}

void PowerManager::enableLoRa()
{
  // IO拡張チップを使用してLoRaの電源をオン
  io.digitalWrite(ExtensionIOXL9555::IO1, HIGH);
  Serial.println("LoRa power enabled");
}

void PowerManager::disableLoRa()
{
  // IO拡張チップを使用してLoRaの電源をオフ
  io.digitalWrite(ExtensionIOXL9555::IO1, LOW);
  Serial.println("LoRa power disabled");
}

void PowerManager::update()
{
  unsigned long currentMillis = millis();

  // バッテリー情報の定期的な更新
  if (currentMillis - lastBatteryCheck >= BATTERY_CHECK_INTERVAL)
  {
    updateBatteryInfo();
    lastBatteryCheck = currentMillis;

    // バッテリー情報のログ出力
    Serial.printf("Battery: %d%%, %dmV, %s\n",
                  batteryLevel,
                  batteryVoltage,
                  charging ? "Charging" : "Discharging");

    // 緊急バッテリー状態の場合、ディープスリープモードに入る
    if (isCriticalBattery() && !charging)
    {
      Serial.println("Critical battery level! Entering deep sleep...");
      enterDeepSleep();
    }
  }
}

void PowerManager::updateBatteryInfo()
{
  // BQ27220からバッテリー情報を取得
  batteryLevel = bq27220.getStateOfCharge();
  batteryVoltage = bq27220.getVoltage();

  // 充電状態の確認
  charging = bq27220.getIsCharging();
}
