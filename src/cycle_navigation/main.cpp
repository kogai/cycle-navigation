#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include "config.h"
#include "power_manager.h"
#include "gps_manager.h"
#include "display_manager.h"
#include "ExtensionIOXL9555.hpp"

// IO拡張チップ
ExtensionIOXL9555 io;

// ボタン状態
bool buttonStates[6] = {false, false, false, false, false, false};
bool lastButtonStates[6] = {false, false, false, false, false, false};

// アプリケーション状態
enum AppState
{
  STATE_INIT,
  STATE_MAP_VIEW,
  STATE_SETTINGS,
  STATE_POWER_SAVE
};

AppState currentState = STATE_INIT;
unsigned long lastUserInteraction = 0;
unsigned long lastGpsUpdate = 0;
unsigned long lastDisplayUpdate = 0;
bool lowPowerMode = false;

// 関数プロトタイプ
void setupButtons();
bool checkButtons();
void handleButtons();
void enterPowerSaveMode();
void exitPowerSaveMode();
void updateDisplay();
void showStatusInfo();

void setup()
{
  // シリアル初期化
  Serial.begin(115200);
  delay(500);
  Serial.println("サイクルナビゲーション起動中...");

  // I2CとSPIの初期化
  Wire.begin(BOARD_SDA, BOARD_SCL);
  SPI.begin(BOARD_SPI_SCLK, BOARD_SPI_MISO, BOARD_SPI_MOSI);

  // SDカードの初期化
  if (!SD.begin(BOARD_SD_CS))
  {
    Serial.println("SDカードの初期化に失敗しました");
  }
  else
  {
    Serial.println("SDカード初期化成功");
  }

  // 電源管理の初期化
  if (!powerManager.init())
  {
    Serial.println("電源管理の初期化に失敗しました");
  }
  else
  {
    Serial.println("電源管理初期化成功");
  }

  // ボタンの設定
  setupButtons();

  // GPSの初期化
  if (!gpsManager.init())
  {
    Serial.println("GPSの初期化に失敗しました");
  }
  else
  {
    Serial.println("GPS初期化成功");
  }

  // ディスプレイの初期化
  if (!displayManager.init())
  {
    Serial.println("ディスプレイの初期化に失敗しました");
  }
  else
  {
    Serial.println("ディスプレイ初期化成功");
  }

  // 初期画面表示
  displayManager.clear();
  displayManager.drawText("サイクルナビゲーション", 100, 100, 3);
  displayManager.drawText("起動中...", 100, 150, 2);
  displayManager.updateScreen();

  delay(2000);

  // 初期マップ表示
  displayManager.drawMap(35.681236, 139.767125, MAP_ZOOM_DEFAULT);

  // 状態を更新
  currentState = STATE_MAP_VIEW;
  lastUserInteraction = millis();
  lastGpsUpdate = millis();
  lastDisplayUpdate = millis();

  Serial.println("初期化完了");
}

void loop()
{
  unsigned long currentMillis = millis();

  // ボタン状態の確認
  if (checkButtons())
  {
    handleButtons();
    lastUserInteraction = currentMillis;

    // 低電力モードから復帰
    if (lowPowerMode)
    {
      exitPowerSaveMode();
    }
  }

  // GPS更新
  if (currentMillis - lastGpsUpdate >= GPS_UPDATE_INTERVAL)
  {
    if (gpsManager.update())
    {
      // 有効な位置情報が得られた場合
      if (gpsManager.isLocationValid())
      {
        // マップ表示モードの場合、現在地を表示
        if (currentState == STATE_MAP_VIEW)
        {
          displayManager.drawLocationMarker(
              gpsManager.getLatitude(),
              gpsManager.getLongitude());
        }
      }
    }
    lastGpsUpdate = currentMillis;
  }

  // 定期的な画面更新
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)
  {
    updateDisplay();
    lastDisplayUpdate = currentMillis;
  }

  // 電源管理の更新
  powerManager.update();

  // 一定時間操作がない場合、低電力モードに移行
  if (!lowPowerMode && currentMillis - lastUserInteraction > 60000)
  {
    enterPowerSaveMode();
  }

  // 緊急バッテリー状態の場合、ディープスリープモードに移行
  if (powerManager.isCriticalBattery() && !powerManager.isCharging())
  {
    Serial.println("緊急バッテリー状態: ディープスリープモードに移行します");
    displayManager.drawText("バッテリー残量低下", 100, 200, 3);
    displayManager.drawText("スリープモードに移行します", 100, 250, 2);
    displayManager.updateScreen();
    delay(2000);
    powerManager.enterDeepSleep();
  }

  delay(10);
}

void setupButtons()
{
  // IO拡張チップの初期化
  if (!io.init(Wire, BOARD_SDA, BOARD_SCL, XL9555_SLAVE_ADDRESS0))
  {
    Serial.println("IO拡張チップの初期化に失敗しました");
    return;
  }

  // IO拡張ポートの設定
  io.configPort(ExtensionIOXL9555::PORT0, 0xFF); // PORT0を入力に設定
  io.configPort(ExtensionIOXL9555::PORT1, 0xFF); // PORT1を入力に設定

  // 割り込みピンの設定
  pinMode(BOARD_PCA9535_INT, INPUT_PULLUP);
}

bool checkButtons()
{
  bool buttonChanged = false;

  // 割り込みピンがLOWの場合、ボタンが押された可能性がある
  if (digitalRead(BOARD_PCA9535_INT) == LOW)
  {
    // PORT0の値を読み取る
    uint8_t port0Value = io.readPort(ExtensionIOXL9555::PORT0);

    // ボタン状態の更新
    for (int i = 0; i < 6; i++)
    {
      bool newState = !(port0Value & (1 << i));
      if (newState != lastButtonStates[i])
      {
        buttonStates[i] = newState;
        lastButtonStates[i] = newState;
        buttonChanged = true;
      }
    }
  }

  return buttonChanged;
}

void handleButtons()
{
  // マップ表示モードの場合
  if (currentState == STATE_MAP_VIEW)
  {
    // 上ボタン: ズームイン
    if (buttonStates[BUTTON_UP])
    {
      displayManager.zoomIn();
    }

    // 下ボタン: ズームアウト
    if (buttonStates[BUTTON_DOWN])
    {
      displayManager.zoomOut();
    }

    // 左ボタン: 左にスクロール
    if (buttonStates[BUTTON_LEFT])
    {
      displayManager.scrollMap(-50, 0);
    }

    // 右ボタン: 右にスクロール
    if (buttonStates[BUTTON_RIGHT])
    {
      displayManager.scrollMap(50, 0);
    }

    // 選択ボタン: 現在地にフォーカス
    if (buttonStates[BUTTON_SELECT])
    {
      if (gpsManager.isLocationValid())
      {
        displayManager.drawMap(
            gpsManager.getLatitude(),
            gpsManager.getLongitude(),
            displayManager.getZoom());
      }
    }

    // 戻るボタン: 設定モードに切り替え
    if (buttonStates[BUTTON_BACK])
    {
      currentState = STATE_SETTINGS;
      // 設定画面の表示
      displayManager.clear();
      displayManager.drawText("設定", 100, 50, 3);
      char modeText[50];
      sprintf(modeText, "1. 低電力モード: %s", lowPowerMode ? "オン" : "オフ");
      displayManager.drawText(modeText, 50, 150, 2);
      displayManager.drawText("2. 戻る", 50, 200, 2);
      displayManager.updateScreen();
    }
  }
  // 設定モードの場合
  else if (currentState == STATE_SETTINGS)
  {
    // 上ボタン: 低電力モード切り替え
    if (buttonStates[BUTTON_UP])
    {
      lowPowerMode = !lowPowerMode;
      if (lowPowerMode)
      {
        gpsManager.enableLowPowerMode();
      }
      else
      {
        gpsManager.disableLowPowerMode();
      }

      // 設定画面の更新
      char modeText[50];
      sprintf(modeText, "1. 低電力モード: %s", lowPowerMode ? "オン" : "オフ");
      displayManager.drawText(modeText, 50, 150, 2);
      displayManager.updateArea(50, 150, 400, 30);
    }

    // 戻るボタン: マップ表示モードに戻る
    if (buttonStates[BUTTON_BACK])
    {
      currentState = STATE_MAP_VIEW;
      // マップ画面に戻る
      displayManager.drawMap(
          gpsManager.isLocationValid() ? gpsManager.getLatitude() : 35.681236,
          gpsManager.isLocationValid() ? gpsManager.getLongitude() : 139.767125,
          displayManager.getZoom());
    }
  }
}

void enterPowerSaveMode()
{
  Serial.println("低電力モードに移行します");
  lowPowerMode = true;
  gpsManager.enableLowPowerMode();

  // 画面に低電力モードを表示
  displayManager.drawText("低電力モード", 100, 50, 2);
  displayManager.drawText("ボタンを押して復帰", 100, 100, 1);
  displayManager.updateArea(100, 50, 200, 100);
}

void exitPowerSaveMode()
{
  Serial.println("通常モードに復帰します");
  lowPowerMode = false;
  gpsManager.disableLowPowerMode();

  // 画面を更新
  updateDisplay();
}

void updateDisplay()
{
  // 現在の状態に応じた表示更新
  if (currentState == STATE_MAP_VIEW)
  {
    // ステータス情報の表示
    showStatusInfo();

    // 現在地が有効な場合、マーカーを更新
    if (gpsManager.isLocationValid())
    {
      displayManager.drawLocationMarker(
          gpsManager.getLatitude(),
          gpsManager.getLongitude());
    }
  }
}

void showStatusInfo()
{
  // バッテリー状態の表示
  displayManager.drawBatteryStatus(
      powerManager.getBatteryLevel(),
      powerManager.isCharging());

  // GPS信号強度の表示
  displayManager.drawGPSSignal(gpsManager.getSignalQuality());

  // 速度の表示（GPS信号が有効な場合）
  if (gpsManager.isLocationValid())
  {
    displayManager.drawSpeed(gpsManager.getSpeed());
    displayManager.drawCourse(gpsManager.getCourse());
  }
}
