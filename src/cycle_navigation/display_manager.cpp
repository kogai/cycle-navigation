#include "display_manager.h"
#include <SD.h>
#include <math.h>
#include "firasans_12.h"
#include "firasans_20.h"

// グローバルインスタンス
DisplayManager displayManager;

// 定数
#define WAVEFORM EPD_BUILTIN_WAVEFORM
#define DEMO_BOARD epd_board_v7

// 方位の定数
const char *DIRECTION_NAMES[] = {"北", "北東", "東", "南東", "南", "南西", "西", "北西"};

DisplayManager::DisplayManager() : currentZoom(MAP_ZOOM_DEFAULT),
                                   centerLatitude(35.681236),   // 東京駅の緯度
                                   centerLongitude(139.767125), // 東京駅の経度
                                   offsetX(0),
                                   offsetY(0)
{
  // タイル配列の初期化
  for (int i = 0; i < 9; i++)
  {
    currentTiles[i].data = nullptr;
    currentTiles[i].loaded = false;
  }
}

bool DisplayManager::init()
{
  // EPDの初期化
  epd_init(&DEMO_BOARD, &ED047TC1, EPD_LUT_64K);

  // VCOMの設定（ソフトウェアでVCOMを設定できるボード用）
  epd_set_vcom(1560);

  // 高レベルAPIの初期化
  hl = epd_hl_init(WAVEFORM);

  // 回転の設定（縦向き）
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);

  Serial.printf(
      "ディスプレイサイズ: 幅 %d, 高さ %d\n",
      epd_rotated_display_width(),
      epd_rotated_display_height());

  // 画面をクリア
  clear();

  return true;
}

void DisplayManager::clear()
{
  epd_poweron();
  epd_clear();
  int temperature = epd_ambient_temperature();
  epd_poweroff();

  Serial.printf("現在の温度: %d\n", temperature);
}

void DisplayManager::updateScreen()
{
  epd_poweron();
  enum EpdDrawError err = epd_hl_update_screen(&hl, MODE_GC16, epd_ambient_temperature());
  if (err != EPD_DRAW_SUCCESS)
  {
    Serial.printf("画面更新エラー: %X\n", err);
  }
  epd_poweroff();
}

void DisplayManager::updateArea(int x, int y, int width, int height)
{
  EpdRect area = {
      .x = x,
      .y = y,
      .width = width,
      .height = height,
  };

  epd_poweron();
  enum EpdDrawError err = epd_hl_update_area(&hl, MODE_DU, epd_ambient_temperature(), area);
  if (err != EPD_DRAW_SUCCESS)
  {
    Serial.printf("領域更新エラー: %X\n", err);
  }
  epd_poweroff();
}

void DisplayManager::drawMap(double centerLat, double centerLon, int zoom)
{
  // 中心座標を更新
  centerLatitude = centerLat;
  centerLongitude = centerLon;
  currentZoom = zoom;

  // 中心タイルの座標を計算
  int centerTileX, centerTileY, pixelX, pixelY;
  latLonToTile(centerLat, centerLon, zoom, centerTileX, centerTileY, pixelX, pixelY);

  // 3x3のタイル配列を読み込む
  for (int y = 0; y < 3; y++)
  {
    for (int x = 0; x < 3; x++)
    {
      int tileX = centerTileX + (x - 1);
      int tileY = centerTileY + (y - 1);
      int index = y * 3 + x;

      // タイルの読み込み
      if (loadMapTile(zoom, tileX, tileY))
      {
        // タイルの描画位置を計算
        int drawX = (x - 1) * MAP_TILE_SIZE - pixelX + epd_rotated_display_width() / 2;
        int drawY = (y - 1) * MAP_TILE_SIZE - pixelY + epd_rotated_display_height() / 2;

        // タイルを描画
        drawMapTile(currentTiles[index], drawX, drawY);
      }
    }
  }

  // スケールバーと方位記号を描画
  drawScaleBar(20, epd_rotated_display_height() - 30);
  drawCompass(epd_rotated_display_width() - 50, 50, 30);

  // 画面を更新
  updateScreen();
}

void DisplayManager::drawLocationMarker(double lat, double lon)
{
  // 緯度経度からピクセル座標に変換
  int tileX, tileY, pixelX, pixelY;
  latLonToTile(lat, lon, currentZoom, tileX, tileY, pixelX, pixelY);

  // 中心タイルからの相対位置を計算
  int centerTileX, centerTileY, centerPixelX, centerPixelY;
  latLonToTile(centerLatitude, centerLongitude, currentZoom, centerTileX, centerTileY, centerPixelX, centerPixelY);

  int relativeX = (tileX - centerTileX) * MAP_TILE_SIZE + pixelX - centerPixelX + epd_rotated_display_width() / 2;
  int relativeY = (tileY - centerTileY) * MAP_TILE_SIZE + pixelY - centerPixelY + epd_rotated_display_height() / 2;

  // マーカーを描画（円と十字）
  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // 外側の円（白）
  epd_fill_circle(relativeX, relativeY, 12, 0xFF, fb);

  // 内側の円（黒）
  epd_fill_circle(relativeX, relativeY, 10, 0, fb);

  // 中心の円（白）
  epd_fill_circle(relativeX, relativeY, 4, 0xFF, fb);

  // 十字（白）
  epd_draw_hline(relativeX - 15, relativeY, 30, 0xFF, fb);
  epd_draw_vline(relativeX, relativeY - 15, 30, 0xFF, fb);

  // 更新領域
  EpdRect updateRect = {
      .x = relativeX - 20,
      .y = relativeY - 20,
      .width = 40,
      .height = 40,
  };

  // 画面を部分更新
  updateArea(updateRect.x, updateRect.y, updateRect.width, updateRect.height);
}

void DisplayManager::drawText(const char *text, int x, int y, int size)
{
  const EpdFont *font;

  // フォントサイズの選択
  switch (size)
  {
  case 1:
    font = &FiraSans_12;
    break;
  case 3:
    font = &FiraSans_20;
    break;
  case 2:
  default:
    font = &FiraSans_12;
    break;
  }

  // テキスト描画のプロパティ設定
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_LEFT;

  // テキストを描画
  uint8_t *fb = epd_hl_get_framebuffer(&hl);
  int cursor_x = x;
  int cursor_y = y;
  epd_write_string(font, text, &cursor_x, &cursor_y, fb, &font_props);
}

void DisplayManager::drawBatteryStatus(int level, bool isCharging)
{
  // バッテリーアイコンの描画位置
  int x = 20;
  int y = 20;
  int width = 40;
  int height = 20;
  int margin = 2;

  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // バッテリー外枠
  epd_draw_rect(EpdRect{x, y, width, height}, 0, fb);

  // バッテリー端子
  epd_fill_rect(EpdRect{x + width, y + height / 4, 3, height / 2}, 0, fb);

  // バッテリー残量
  int fillWidth = (width - margin * 2) * level / 100;
  epd_fill_rect(EpdRect{x + margin, y + margin, fillWidth, height - margin * 2}, 0, fb);

  // 充電中アイコン
  if (isCharging)
  {
    // 稲妻マーク
    int cx = x + width / 2;
    int cy = y + height / 2;
    int boltSize = height - margin * 4;

    // 簡易的な稲妻マーク
    int boltPoints[] = {
        cx, cy - boltSize / 2,
        cx + boltSize / 3, cy,
        cx, cy,
        cx, cy + boltSize / 2,
        cx - boltSize / 3, cy,
        cx, cy};

    for (int i = 0; i < 5; i++)
    {
      epd_draw_line(
          boltPoints[i * 2], boltPoints[i * 2 + 1],
          boltPoints[(i + 1) * 2], boltPoints[(i + 1) * 2 + 1],
          0xFF, fb);
    }
  }

  // バッテリー残量テキスト
  char levelText[10];
  sprintf(levelText, "%d%%", level);
  drawText(levelText, x + width + 10, y + height / 2, 1);

  // 更新領域
  EpdRect updateRect = {
      .x = x - 5,
      .y = y - 5,
      .width = width + 50,
      .height = height + 10,
  };

  // 画面を部分更新
  updateArea(updateRect.x, updateRect.y, updateRect.width, updateRect.height);
}

void DisplayManager::drawGPSSignal(int quality)
{
  // GPS信号アイコンの描画位置
  int x = 100;
  int y = 20;
  int width = 20;
  int height = 20;
  int barWidth = 3;
  int barGap = 2;

  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // 信号強度バーを描画
  for (int i = 0; i < 4; i++)
  {
    int barHeight = (i + 1) * 5;
    int barX = x + i * (barWidth + barGap);
    int barY = y + height - barHeight;

    // 信号強度に応じて塗りつぶし
    uint8_t color = (i < quality) ? 0 : 0xAA;
    epd_fill_rect(EpdRect{barX, barY, barWidth, barHeight}, color, fb);
  }

  // GPS信号テキスト
  char qualityText[20];
  if (quality == 0)
  {
    sprintf(qualityText, "GPS: なし");
  }
  else
  {
    sprintf(qualityText, "GPS: %d/4", quality);
  }
  drawText(qualityText, x + 30, y + height / 2, 1);

  // 更新領域
  EpdRect updateRect = {
      .x = x - 5,
      .y = y - 5,
      .width = 100,
      .height = height + 10,
  };

  // 画面を部分更新
  updateArea(updateRect.x, updateRect.y, updateRect.width, updateRect.height);
}

void DisplayManager::drawSpeed(double speed)
{
  // 速度表示の位置
  int x = 20;
  int y = epd_rotated_display_height() - 80;

  char speedText[20];
  sprintf(speedText, "%.1f km/h", speed);

  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // 背景を白で塗りつぶし
  epd_fill_rect(EpdRect{x - 5, y - 5, 120, 30}, 0xFF, fb);

  // 速度テキストを描画
  drawText(speedText, x, y, 3);

  // 更新領域
  EpdRect updateRect = {
      .x = x - 10,
      .y = y - 10,
      .width = 130,
      .height = 40,
  };

  // 画面を部分更新
  updateArea(updateRect.x, updateRect.y, updateRect.width, updateRect.height);
}

void DisplayManager::drawCourse(double course)
{
  // 方位表示の位置
  int x = epd_rotated_display_width() - 100;
  int y = epd_rotated_display_height() - 80;

  // 方位名を取得
  int directionIndex = (int)round(course / 45.0) % 8;
  const char *directionName = DIRECTION_NAMES[directionIndex];

  char courseText[20];
  sprintf(courseText, "%s %.0f°", directionName, course);

  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // 背景を白で塗りつぶし
  epd_fill_rect(EpdRect{x - 5, y - 5, 100, 30}, 0xFF, fb);

  // 方位テキストを描画
  drawText(courseText, x, y, 2);

  // 更新領域
  EpdRect updateRect = {
      .x = x - 10,
      .y = y - 10,
      .width = 110,
      .height = 40,
  };

  // 画面を部分更新
  updateArea(updateRect.x, updateRect.y, updateRect.width, updateRect.height);
}

void DisplayManager::setZoom(int zoom)
{
  // ズームレベルの制限（通常は0-19）
  if (zoom < 0)
    zoom = 0;
  if (zoom > 19)
    zoom = 19;

  currentZoom = zoom;
}

int DisplayManager::getZoom()
{
  return currentZoom;
}

void DisplayManager::zoomIn()
{
  if (currentZoom < 19)
  {
    currentZoom++;
    drawMap(centerLatitude, centerLongitude, currentZoom);
  }
}

void DisplayManager::zoomOut()
{
  if (currentZoom > 0)
  {
    currentZoom--;
    drawMap(centerLatitude, centerLongitude, currentZoom);
  }
}

void DisplayManager::scrollMap(int deltaX, int deltaY)
{
  // スクロール量からピクセル単位での移動を計算
  offsetX += deltaX;
  offsetY += deltaY;

  // 一定以上スクロールしたら中心座標を更新
  if (abs(offsetX) > MAP_TILE_SIZE / 4 || abs(offsetY) > MAP_TILE_SIZE / 4)
  {
    // 現在の中心座標からのピクセル移動量を計算
    double pixelsPerLonDegree = MAP_TILE_SIZE * (1 << currentZoom) / 360.0;
    double pixelsPerLatRadian = MAP_TILE_SIZE * (1 << currentZoom) / (2 * M_PI);

    // 緯度経度の変化量を計算
    double lonDelta = offsetX / pixelsPerLonDegree;
    double latDelta = -offsetY / pixelsPerLatRadian;

    // 新しい中心座標を計算
    centerLongitude += lonDelta;
    double newLat = centerLatitude + latDelta;

    // 緯度の範囲を制限（-85.05〜85.05）
    if (newLat < -85.05)
      newLat = -85.05;
    if (newLat > 85.05)
      newLat = 85.05;
    centerLatitude = newLat;

    // オフセットをリセット
    offsetX = 0;
    offsetY = 0;

    // マップを再描画
    drawMap(centerLatitude, centerLongitude, currentZoom);
  }
}

void DisplayManager::latLonToTile(double lat, double lon, int zoom, int &x, int &y, int &pixelX, int &pixelY)
{
  // 緯度経度をタイル座標に変換（OpenStreetMapの計算式）
  double n = pow(2.0, zoom);

  // タイルのX座標を計算
  double tileX = ((lon + 180.0) / 360.0) * n;
  x = floor(tileX);
  pixelX = floor((tileX - x) * MAP_TILE_SIZE);

  // タイルのY座標を計算
  double latRad = lat * M_PI / 180.0;
  double tileY = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) * n / 2.0;
  y = floor(tileY);
  pixelY = floor((tileY - y) * MAP_TILE_SIZE);
}

void DisplayManager::tileToLatLon(int x, int y, int zoom, int pixelX, int pixelY, double &lat, double &lon)
{
  // タイル座標を緯度経度に変換（OpenStreetMapの計算式）
  double n = pow(2.0, zoom);

  // 経度を計算
  lon = (x + pixelX / (double)MAP_TILE_SIZE) / n * 360.0 - 180.0;

  // 緯度を計算
  double tileY = y + pixelY / (double)MAP_TILE_SIZE;
  double latRad = atan(sinh(M_PI * (1 - 2 * tileY / n)));
  lat = latRad * 180.0 / M_PI;
}

bool DisplayManager::loadMapTile(int zoom, int x, int y)
{
  // タイル配列のインデックスを計算
  int index = (y % 3) * 3 + (x % 3);

  // すでに同じタイルが読み込まれている場合はスキップ
  if (currentTiles[index].loaded &&
      currentTiles[index].zoom == zoom &&
      currentTiles[index].x == x &&
      currentTiles[index].y == y)
  {
    return true;
  }

  // 以前のタイルデータを解放
  if (currentTiles[index].data != nullptr)
  {
    free(currentTiles[index].data);
    currentTiles[index].data = nullptr;
  }

  // タイルのファイルパスを構築
  char filePath[64];
  sprintf(filePath, "%s/%d/%d/%d.png", SD_MAP_DIRECTORY, zoom, x, y);

  // SDカードからタイルを読み込む
  if (!SD.exists(filePath))
  {
    Serial.printf("タイルが見つかりません: %s\n", filePath);

    // タイルが見つからない場合は空のタイルを作成
    currentTiles[index].zoom = zoom;
    currentTiles[index].x = x;
    currentTiles[index].y = y;
    currentTiles[index].loaded = false;

    return false;
  }

  File tileFile = SD.open(filePath);
  if (!tileFile)
  {
    Serial.printf("タイルを開けません: %s\n", filePath);
    return false;
  }

  // タイルのサイズを取得
  size_t fileSize = tileFile.size();

  // メモリを確保
  currentTiles[index].data = (uint8_t *)ps_malloc(MAP_TILE_SIZE * MAP_TILE_SIZE);
  if (currentTiles[index].data == nullptr)
  {
    Serial.println("タイル用のメモリを確保できません");
    tileFile.close();
    return false;
  }

  // ここでは簡易的な実装として、PNGファイルをそのまま読み込む代わりに
  // グレースケールの単純なパターンを生成
  for (int py = 0; py < MAP_TILE_SIZE; py++)
  {
    for (int px = 0; px < MAP_TILE_SIZE; px++)
    {
      // 市松模様のパターン
      bool isEvenX = ((x + px / 32) % 2) == 0;
      bool isEvenY = ((y + py / 32) % 2) == 0;

      if (isEvenX == isEvenY)
      {
        currentTiles[index].data[py * MAP_TILE_SIZE + px] = 0xCC; // 薄いグレー
      }
      else
      {
        currentTiles[index].data[py * MAP_TILE_SIZE + px] = 0x88; // 濃いグレー
      }

      // 境界線を描画
      if (px == 0 || px == MAP_TILE_SIZE - 1 || py == 0 || py == MAP_TILE_SIZE - 1)
      {
        currentTiles[index].data[py * MAP_TILE_SIZE + px] = 0x44; // 境界線
      }
    }
  }

  tileFile.close();

  // タイル情報を更新
  currentTiles[index].zoom = zoom;
  currentTiles[index].x = x;
  currentTiles[index].y = y;
  currentTiles[index].loaded = true;

  return true;
}

void DisplayManager::drawMapTile(const MapTile &tile, int x, int y)
{
  // タイルが読み込まれていない場合
  if (!tile.loaded || tile.data == nullptr)
  {
    // 空のタイルを描画（グリッド表示）
    uint8_t *fb = epd_hl_get_framebuffer(&hl);

    // タイル全体を薄いグレーで塗りつぶし
    epd_fill_rect(EpdRect{x, y, MAP_TILE_SIZE, MAP_TILE_SIZE}, 0xEE, fb);

    // グリッド線を描画
    for (int i = 0; i <= MAP_TILE_SIZE; i += 32)
    {
      epd_draw_hline(x, y + i, MAP_TILE_SIZE, 0xAA, fb);
      epd_draw_vline(x + i, y, MAP_TILE_SIZE, 0xAA, fb);
    }

    // タイル座標を表示
    char coordText[32];
    sprintf(coordText, "Z%d/%d/%d", tile.zoom, tile.x, tile.y);

    int textX = x + MAP_TILE_SIZE / 2 - 30;
    int textY = y + MAP_TILE_SIZE / 2;
    drawText(coordText, textX, textY, 1);

    return;
  }

  // タイルデータを描画
  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // タイルの表示領域
  EpdRect tileRect = {
      .x = x,
      .y = y,
      .width = MAP_TILE_SIZE,
      .height = MAP_TILE_SIZE,
  };

  // タイルデータをフレームバッファにコピー
  // 実際のアプリケーションでは、PNGデコードやその他の処理が必要
  for (int py = 0; py < MAP_TILE_SIZE; py++)
  {
    for (int px = 0; px < MAP_TILE_SIZE; px++)
    {
      int fbX = x + px;
      int fbY = y + py;

      // 画面の範囲内かチェック
      if (fbX >= 0 && fbX < epd_rotated_display_width() &&
          fbY >= 0 && fbY < epd_rotated_display_height())
      {
        int fbOffset = fbY * epd_rotated_display_width() + fbX;
        int tileOffset = py * MAP_TILE_SIZE + px;

        fb[fbOffset] = tile.data[tileOffset];
      }
    }
  }
}

void DisplayManager::drawCompass(int x, int y, int size)
{
  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // 外側の円
  epd_draw_circle(x, y, size, 0, fb);

  // 方位の線
  for (int i = 0; i < 8; i++)
  {
    double angle = i * M_PI / 4.0;
    int endX = x + size * 0.8 * sin(angle);
    int endY = y - size * 0.8 * cos(angle);

    epd_draw_line(x, y, endX, endY, 0, fb);
  }

  // 方位のラベル
  const char *directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  for (int i = 0; i < 8; i++)
  {
    double angle = i * M_PI / 4.0;
    int labelX = x + size * 0.9 * sin(angle);
    int labelY = y - size * 0.9 * cos(angle);

    // ラベルの位置調整
    int textX = labelX - 5;
    int textY = labelY - 5;

    drawText(directions[i], textX, textY, 1);
  }
}

void DisplayManager::drawScaleBar(int x, int y)
{
  // 現在のズームレベルでの縮尺を計算
  // 赤道上での1ピクセルあたりの距離（メートル）
  double metersPerPixel = 156543.03392 * cos(centerLatitude * M_PI / 180.0) / pow(2, currentZoom);

  // スケールバーの長さを決定（100m、200m、500m、1km、2km、5km、10km、...）
  int scaleBarLengthMeters;
  int scaleBarPixels;

  if (metersPerPixel * 100 < 100)
  {
    scaleBarLengthMeters = 100;
  }
  else if (metersPerPixel * 100 < 200)
  {
    scaleBarLengthMeters = 200;
  }
  else if (metersPerPixel * 100 < 500)
  {
    scaleBarLengthMeters = 500;
  }
  else if (metersPerPixel * 100 < 1000)
  {
    scaleBarLengthMeters = 1000;
  }
  else if (metersPerPixel * 100 < 2000)
  {
    scaleBarLengthMeters = 2000;
  }
  else if (metersPerPixel * 100 < 5000)
  {
    scaleBarLengthMeters = 5000;
  }
  else
  {
    scaleBarLengthMeters = 10000;
  }

  // スケールバーのピクセル長を計算
  scaleBarPixels = scaleBarLengthMeters / metersPerPixel;

  // スケールバーを描画
  uint8_t *fb = epd_hl_get_framebuffer(&hl);

  // バーの背景
  epd_fill_rect(EpdRect{x - 5, y - 15, scaleBarPixels + 10, 25}, 0xFF, fb);

  // バー
  epd_fill_rect(EpdRect{x, y, scaleBarPixels, 5}, 0, fb);

  // 端の縦線
  epd_fill_rect(EpdRect{x, y - 5, 2, 15}, 0, fb);
  epd_fill_rect(EpdRect{x + scaleBarPixels - 2, y - 5, 2, 15}, 0, fb);

  // スケールのテキスト
  char scaleText[20];
  if (scaleBarLengthMeters >= 1000)
  {
    sprintf(scaleText, "%d km", scaleBarLengthMeters / 1000);
  }
  else
  {
    sprintf(scaleText, "%d m", scaleBarLengthMeters);
  }

  drawText(scaleText, x + scaleBarPixels / 2 - 15, y - 10, 1);
}
