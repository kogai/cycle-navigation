#pragma once

// ボードのピン定義
#define BOARD_GPS_RXD 44
#define BOARD_GPS_TXD 43
#define BOARD_GPS_PPS 1
#define SerialMon Serial
#define SerialGPS Serial1

#define BOARD_I2C_PORT (0)
#define BOARD_SCL (40)
#define BOARD_SDA (39)

#define BOARD_SPI_MISO (21)
#define BOARD_SPI_MOSI (13)
#define BOARD_SPI_SCLK (14)

#define BOARD_TOUCH_SCL (BOARD_SCL)
#define BOARD_TOUCH_SDA (BOARD_SDA)
#define BOARD_TOUCH_INT (3)
#define BOARD_TOUCH_RST (9)

#define BOARD_RTC_SCL (BOARD_SCL)
#define BOARD_RTC_SDA (BOARD_SDA)
#define BOARD_RTC_IRQ (2)

#define BOARD_SD_MISO (BOARD_SPI_MISO)
#define BOARD_SD_MOSI (BOARD_SPI_MOSI)
#define BOARD_SD_SCLK (BOARD_SPI_SCLK)
#define BOARD_SD_CS (12)

#define BOARD_LORA_MISO (BOARD_SPI_MISO)
#define BOARD_LORA_MOSI (BOARD_SPI_MOSI)
#define BOARD_LORA_SCLK (BOARD_SPI_SCLK)
#define BOARD_LORA_CS (46)
#define BOARD_LORA_IRQ (10)
#define BOARD_LORA_RST (1)
#define BOARD_LORA_BUSY (47)

#define BOARD_BL_EN (11)
#define BOARD_PCA9535_INT (38)
#define BOARD_BOOT_BTN (0)

// 物理ボタン定義
// IO拡張チップ（PCA9535PW）を使用したボタン
#define BUTTON_UP 0     // 上方向ボタン（IO拡張ポート番号）
#define BUTTON_DOWN 1   // 下方向ボタン（IO拡張ポート番号）
#define BUTTON_LEFT 2   // 左方向ボタン（IO拡張ポート番号）
#define BUTTON_RIGHT 3  // 右方向ボタン（IO拡張ポート番号）
#define BUTTON_SELECT 4 // 選択ボタン（IO拡張ポート番号）
#define BUTTON_BACK 5   // 戻るボタン（IO拡張ポート番号）

// GPS設定
#define GPS_UPDATE_INTERVAL 5000 // GPS更新間隔（ミリ秒）
#define GPS_SLEEP_INTERVAL 55000 // GPS休止間隔（ミリ秒）

// 電源管理設定
#define BATTERY_CHECK_INTERVAL 60000 // バッテリー残量チェック間隔（ミリ秒）
#define LOW_BATTERY_THRESHOLD 15     // 低バッテリー警告閾値（%）
#define CRITICAL_BATTERY_THRESHOLD 5 // 緊急バッテリー閾値（%）

// ディスプレイ設定
#define DISPLAY_UPDATE_INTERVAL 30000 // ディスプレイ更新間隔（ミリ秒）
#define MAP_ZOOM_DEFAULT 15           // デフォルトのズームレベル
#define MAP_TILE_SIZE 256             // マップタイルのサイズ（ピクセル）

// SDカード設定
#define SD_MAP_DIRECTORY "/maps"      // マップタイルを保存するディレクトリ
#define SD_CONFIG_FILE "/config.json" // 設定ファイル

// システム設定
#define DEEP_SLEEP_DURATION 10000000 // ディープスリープ時間（マイクロ秒）
#define WAKE_UP_PIN GPIO_NUM_0       // ウェイクアップピン（BOOT_BTN）
