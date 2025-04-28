/**
 * サイクルナビゲーション - Hello World
 *
 * 最も基本的な実装から始める
 */

#include <Arduino.h>
#include <Wire.h>
#include <epdiy.h>
#include "config.h"

// 使用するフォントを定義
#include "firasans_12.h"

// 使用するボードとディスプレイを定義
#define DEMO_BOARD epd_board_v7
#define WAVEFORM EPD_BUILTIN_WAVEFORM

// ハイレベル状態の変数
EpdiyHighlevelState hl;

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

  // E-Paperディスプレイの初期化
  epd_init(&DEMO_BOARD, &ED047TC1, EPD_LUT_64K);

  // VCOMの設定（ハードウェアポテンショメータで設定されている場合は不要）
  epd_set_vcom(1560);

  // ハイレベルインターフェースの初期化
  hl = epd_hl_init(WAVEFORM);

  // 回転の設定（デフォルトはEPD_ROT_LANDSCAPE）
  epd_set_rotation(EPD_ROT_PORTRAIT);

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
  delay(1000);
}
