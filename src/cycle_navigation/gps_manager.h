#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>
#include "config.h"

class GPSManager
{
public:
  GPSManager();

  /**
   * @brief 初期化処理
   * @return 初期化成功でtrue、失敗でfalse
   */
  bool init();

  /**
   * @brief GPSデータの更新
   * @return 有効な位置情報が取得できた場合true
   */
  bool update();

  /**
   * @brief 緯度を取得
   * @return 緯度（度）
   */
  double getLatitude();

  /**
   * @brief 経度を取得
   * @return 経度（度）
   */
  double getLongitude();

  /**
   * @brief 高度を取得
   * @return 高度（メートル）
   */
  double getAltitude();

  /**
   * @brief 速度を取得
   * @return 速度（km/h）
   */
  double getSpeed();

  /**
   * @brief 進行方向を取得
   * @return 進行方向（度、北が0度）
   */
  double getCourse();

  /**
   * @brief 衛星数を取得
   * @return 捕捉している衛星数
   */
  int getSatellites();

  /**
   * @brief 最後に位置情報を更新した時間を取得
   * @return 最終更新時間（ミリ秒）
   */
  unsigned long getLastUpdateTime();

  /**
   * @brief GPS信号の品質を取得
   * @return 0-4の値（0:無効、4:最高品質）
   */
  int getSignalQuality();

  /**
   * @brief 省電力モードを有効にする
   */
  void enableLowPowerMode();

  /**
   * @brief 省電力モードを無効にする
   */
  void disableLowPowerMode();

  /**
   * @brief 位置情報が有効かどうかを確認
   * @return 有効な位置情報があればtrue
   */
  bool isLocationValid();

private:
  TinyGPSPlus gps;
  unsigned long lastUpdateTime;
  unsigned long lastGPSRead;
  bool lowPowerMode;
  bool locationValid;

  // GPS位置情報
  double latitude;
  double longitude;
  double altitude;
  double speed;
  double course;
  int satellites;
  int signalQuality;

  /**
   * @brief GPSモジュールの初期化
   * @return 初期化成功でtrue
   */
  bool setupGPS();

  /**
   * @brief GPSモジュールのリカバリー処理
   * @return リカバリー成功でtrue
   */
  bool gpsRecovery();
};

extern GPSManager gpsManager;
