/**
 * サイクルナビゲーション - Hello World
 *
 * 最も基本的な実装から始める
 */

#include <Arduino.h>
#include <Wire.h>
#include <epdiy.h>
#include "config.h"

// バッテリー関連のライブラリ
#include "bq27220.h"
#define XPOWERS_CHIP_BQ25896
#include <XPowersLib.h>

// 定数定義
#define BQ25896_SLAVE_ADDRESS 0x6B
#define SerialMon Serial
#define BATTERY_CHECK_INTERVAL 60000 // バッテリー残量チェック間隔（ミリ秒）

// 使用するフォントを定義
#include "firasans_12.h"

// 使用するボードとディスプレイを定義
#define DEMO_BOARD epd_board_v7
#define WAVEFORM EPD_BUILTIN_WAVEFORM

// ハイレベル状態の変数
EpdiyHighlevelState hl;

// バッテリー関連のオブジェクト
BQ27220 bq27220;
XPowersPPM PPM;

// バッテリー残量表示用の変数
uint16_t batteryPercent = 0;
uint16_t batteryVoltage = 0;

// バッテリー残量を取得する関数
void updateBatteryStatus()
{
  if (bq27220.init())
  {
    batteryPercent = bq27220.getStateOfCharge();
    batteryVoltage = bq27220.getVoltage();
  }
}

// バッテリー残量を表示する関数
void displayBatteryStatus(uint8_t *framebuffer)
{
  char batteryText[32];
  snprintf(batteryText, sizeof(batteryText), "バッテリー: %d%% %dmV", batteryPercent, batteryVoltage);

  // 画面右上に表示
  int cursor_x = epd_rotated_display_width() - 10;
  int cursor_y = 20;

  // フォントプロパティの設定（右揃え）
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_RIGHT;

  // バッテリー情報の描画
  epd_write_string(&FiraSans_12, batteryText, &cursor_x, &cursor_y, framebuffer, &font_props);
}

void setup()
{
  // シリアル通信の初期化
  SerialMon.begin(115200);
  delay(1000); // 安定化のための遅延

  // Hello Worldメッセージの表示（シリアル）
  SerialMon.println("Hello World!");
  SerialMon.println("サイクルナビゲーション - 初期テスト");

  // I2Cの初期化
  Wire.begin(BOARD_SDA, BOARD_SCL);

  // バッテリー関連の初期化
  bool bq27220_initialized = bq27220.init();
  bool ppm_initialized = PPM.init(Wire, BOARD_SDA, BOARD_SCL, BQ25896_SLAVE_ADDRESS);

  if (bq27220_initialized)
  {
    SerialMon.println("BQ27220初期化成功");
    updateBatteryStatus();
  }
  else
  {
    SerialMon.println("BQ27220初期化失敗");
  }

  if (ppm_initialized)
  {
    SerialMon.println("BQ25896初期化成功");
    PPM.enableMeasure(); // 電圧データを取得するためにADCを有効化
  }
  else
  {
    SerialMon.println("BQ25896初期化失敗");
  }

  // E-Paperディスプレイの初期化
  epd_init(&DEMO_BOARD, &ED047TC1, EPD_LUT_64K);

  // VCOMの設定（ハードウェアポテンショメータで設定されている場合は不要）
  epd_set_vcom(1560);

  // ハイレベルインターフェースの初期化
  hl = epd_hl_init(WAVEFORM);

  // 回転の設定（デフォルトはEPD_ROT_LANDSCAPE）
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);

  // 回転後のディスプレイサイズを表示
  SerialMon.printf("ディスプレイサイズ: 幅 %d, 高さ %d\n",
                   epd_rotated_display_width(),
                   epd_rotated_display_height());

  // フレームバッファの取得
  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // 画面をクリア
  epd_poweron();
  epd_clear();
  epd_poweroff();

  // 現在の温度を取得
  int temperature = epd_ambient_temperature();
  SerialMon.printf("現在の温度: %d\n", temperature);

  // テキスト表示の設定
  int cursor_x = epd_rotated_display_width() / 2;
  int cursor_y = epd_rotated_display_height() / 2;

  // フォントプロパティの設定（中央揃え）
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_CENTER;

  // "Hello World"テキストの描画
  epd_write_string(&FiraSans_12, "Hello World", &cursor_x, &cursor_y, fb, &font_props);

  // バッテリー残量の表示
  displayBatteryStatus(fb);

  // 画面の更新
  epd_poweron();
  enum EpdDrawError err = epd_hl_update_screen(&hl, MODE_GL16, temperature);
  if (err != EPD_DRAW_SUCCESS)
  {
    SerialMon.printf("描画エラー: %X\n", err);
  }
  epd_poweroff();

  SerialMon.println("E-Paperディスプレイに「Hello World」を表示しました");
}

void loop()
{
  // 1秒ごとにメッセージを表示（シリアルのみ）
  SerialMon.println("動作中...");

  // 60秒ごとにバッテリー残量を更新して表示
  static unsigned long lastBatteryUpdate = 0;
  if (millis() - lastBatteryUpdate > BATTERY_CHECK_INTERVAL)
  {
    lastBatteryUpdate = millis();

    // バッテリー残量の更新
    updateBatteryStatus();

    // フレームバッファの取得
    uint8_t *fb = epd_hl_get_framebuffer(&hl);

    // 画面をクリア
    epd_poweron();
    epd_clear();
    epd_poweroff();

    // テキスト表示の設定
    int cursor_x = epd_rotated_display_width() / 2;
    int cursor_y = epd_rotated_display_height() / 2;

    // フォントプロパティの設定（中央揃え）
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;

    // "Hello World"テキストの描画
    epd_write_string(&FiraSans_12, "Hello World", &cursor_x, &cursor_y, fb, &font_props);

    // バッテリー残量の表示
    displayBatteryStatus(fb);

    // 画面の更新
    int temperature = epd_ambient_temperature();
    epd_poweron();
    enum EpdDrawError err = epd_hl_update_screen(&hl, MODE_GL16, temperature);
    if (err != EPD_DRAW_SUCCESS)
    {
      SerialMon.printf("描画エラー: %X\n", err);
    }
    epd_poweroff();

    SerialMon.printf("バッテリー残量: %d%%, 電圧: %dmV\n", batteryPercent, batteryVoltage);
  }

  delay(1000);
}
