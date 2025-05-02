/**
 * ディスプレイ管理モジュール実装
 */

#include "display.h"
#include "../firasans_12.h"

// コンストラクタ
Display::Display() : initialized(false)
{
}

// 初期化
bool Display::init()
{
  // E-Paperディスプレイの初期化
  epd_init(&epd_board_v7, &ED047TC1, EPD_LUT_64K);

  // VCOMの設定（ハードウェアポテンショメータで設定されている場合は不要）
  epd_set_vcom(1560);

  // ハイレベルインターフェースの初期化
  hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);

  // 回転の設定（デフォルトはEPD_ROT_LANDSCAPE）
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);

  initialized = true;
  return initialized;
}

// 画面をクリア
void Display::clearScreen()
{
  if (!initialized)
    return;

  epd_hl_set_all_white(&hl);
  // epd_poweron();
  epd_clear();
  // epd_poweroff();
}

// テキストを中央に表示
void Display::displayCenteredText(const char *text, const EpdFont *font)
{
  if (!initialized)
    return;

  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // テキスト表示の設定
  int cursor_x = epd_rotated_display_width() / 2;
  int cursor_y = epd_rotated_display_height() / 2;

  // フォントプロパティの設定（中央揃え）
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_CENTER;

  // テキストの描画
  epd_write_string(font, text, &cursor_x, &cursor_y, fb, &font_props);
}

// バッテリー情報を表示
void Display::displayBatteryStatus(Battery &battery, const EpdFont *font)
{
  if (!initialized)
    return;

  uint8_t *fb = epd_hl_get_framebuffer(&hl);
  char batteryText[32];
  uint16_t batteryPercent = battery.getPercent();
  uint16_t batteryVoltage = battery.getVoltage();

  snprintf(batteryText, sizeof(batteryText), "Battery: %d%% %dmV", batteryPercent, batteryVoltage);

  // 画面右上に表示
  int cursor_x = epd_rotated_display_width() - 10;
  int cursor_y = 30;

  // フォントプロパティの設定（右揃え）
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_RIGHT;

  // バッテリー情報の描画
  epd_write_string(font, batteryText, &cursor_x, &cursor_y, fb, &font_props);
}

// 画面を更新
void Display::updateScreen()
{
  if (!initialized)
    return;

  epd_poweron();
  epd_clear();
  enum EpdDrawError err = epd_hl_update_screen(&hl, MODE_GL16, epd_ambient_temperature());
  if (err != EPD_DRAW_SUCCESS)
  {
    SerialMon.printf("描画エラーが発生しました %d\n", err);
  }
  epd_poweroff();
}

// TODO: 多分要らない
// フレームバッファを取得
uint8_t *Display::getFramebuffer()
{
  if (!initialized)
    return nullptr;
  return epd_hl_get_framebuffer(&hl);
}

// 回転後のディスプレイ幅を取得
int Display::getWidth()
{
  return epd_rotated_display_width();
}

// 回転後のディスプレイ高さを取得
int Display::getHeight()
{
  return epd_rotated_display_height();
}
