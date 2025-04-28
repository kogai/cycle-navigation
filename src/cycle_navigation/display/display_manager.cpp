/**
 * ディスプレイ管理モジュール実装
 */

#include "display_manager.h"
#include "../firasans_12.h"

// コンストラクタ
DisplayManager::DisplayManager() : initialized(false)
{
}

// 初期化
bool DisplayManager::init()
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
void DisplayManager::clearScreen()
{
  if (!initialized)
    return;

  epd_poweron();
  epd_clear();
  epd_poweroff();
}

// テキストを中央に表示
void DisplayManager::displayCenteredText(const char *text, const EpdFont *font)
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
void DisplayManager::displayBatteryStatus(BatteryManager &battery, const EpdFont *font)
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
void DisplayManager::updateScreen()
{
  if (!initialized)
    return;

  // 現在の温度を取得
  int temperature = epd_ambient_temperature();

  // 画面の更新
  epd_poweron();
  enum EpdDrawError err = epd_hl_update_screen(&hl, MODE_GL16, temperature);
  epd_poweroff();
}

// フレームバッファを取得
uint8_t *DisplayManager::getFramebuffer()
{
  if (!initialized)
    return nullptr;
  return epd_hl_get_framebuffer(&hl);
}

// 回転後のディスプレイ幅を取得
int DisplayManager::getWidth()
{
  return epd_rotated_display_width();
}

// 回転後のディスプレイ高さを取得
int DisplayManager::getHeight()
{
  return epd_rotated_display_height();
}
