/**
 * バッテリー管理モジュール実装
 */

#include "battery_manager.h"

// コンストラクタ
BatteryManager::BatteryManager() : initialized(false)
{
}

// 初期化
bool BatteryManager::init(TwoWire &wire, int sda, int scl)
{
  // BQ27220の初期化前にbegin()を呼び出す
  wire.begin(sda, scl);

  bool bq27220_initialized = bq27220.init();
  bool ppm_initialized = PPM.init(wire, sda, scl, BQ25896_SLAVE_ADDRESS);

  initialized = bq27220_initialized && ppm_initialized;

  if (ppm_initialized)
  {
    PPM.enableMeasure(); // 電圧データを取得するためにADCを有効化
  }

  return initialized;
}

// バッテリー残量を取得
uint16_t BatteryManager::getPercent()
{
  if (initialized)
  {
    return bq27220.getStateOfCharge();
  }
  return 0;
}

// バッテリー電圧を取得
uint16_t BatteryManager::getVoltage()
{
  if (initialized)
  {
    return bq27220.getVoltage();
  }
  return 0;
}

// バッテリーが初期化されているか確認
bool BatteryManager::isInitialized() const
{
  return initialized;
}
