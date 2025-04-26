#pragma once

#include <Arduino.h>
#include <epdiy.h>
#include "config.h"

// マップタイルの構造体
struct MapTile
{
  int zoom;
  int x;
  int y;
  uint8_t *data;
  bool loaded;
};

class DisplayManager
{
public:
  DisplayManager();

  /**
   * @brief 初期化処理
   * @return 初期化成功でtrue、失敗でfalse
   */
  bool init();

  /**
   * @brief ディスプレイをクリア
   */
  void clear();

  /**
   * @brief 画面全体を更新
   */
  void updateScreen();

  /**
   * @brief 指定領域のみを更新
   * @param x 更新領域のX座標
   * @param y 更新領域のY座標
   * @param width 更新領域の幅
   * @param height 更新領域の高さ
   */
  void updateArea(int x, int y, int width, int height);

  /**
   * @brief マップを描画
   * @param centerLat 中心緯度
   * @param centerLon 中心経度
   * @param zoom ズームレベル
   */
  void drawMap(double centerLat, double centerLon, int zoom);

  /**
   * @brief 現在地マーカーを描画
   * @param lat 緯度
   * @param lon 経度
   */
  void drawLocationMarker(double lat, double lon);

  /**
   * @brief テキストを描画
   * @param text 表示するテキスト
   * @param x X座標
   * @param y Y座標
   * @param size フォントサイズ（1: 小, 2: 中, 3: 大）
   */
  void drawText(const char *text, int x, int y, int size = 2);

  /**
   * @brief バッテリー残量を表示
   * @param level バッテリー残量（%）
   * @param isCharging 充電中かどうか
   */
  void drawBatteryStatus(int level, bool isCharging);

  /**
   * @brief GPS信号強度を表示
   * @param quality 信号強度（0-4）
   */
  void drawGPSSignal(int quality);

  /**
   * @brief 速度を表示
   * @param speed 速度（km/h）
   */
  void drawSpeed(double speed);

  /**
   * @brief 進行方向を表示
   * @param course 進行方向（度）
   */
  void drawCourse(double course);

  /**
   * @brief ズームレベルを設定
   * @param zoom ズームレベル
   */
  void setZoom(int zoom);

  /**
   * @brief ズームレベルを取得
   * @return 現在のズームレベル
   */
  int getZoom();

  /**
   * @brief ズームイン
   */
  void zoomIn();

  /**
   * @brief ズームアウト
   */
  void zoomOut();

  /**
   * @brief マップをスクロール
   * @param deltaX X方向のスクロール量
   * @param deltaY Y方向のスクロール量
   */
  void scrollMap(int deltaX, int deltaY);

private:
  EpdiyHighlevelState hl;
  int currentZoom;
  double centerLatitude;
  double centerLongitude;
  int offsetX;
  int offsetY;
  MapTile currentTiles[9]; // 3x3のタイル配列

  /**
   * @brief 緯度経度からタイル座標に変換
   * @param lat 緯度
   * @param lon 経度
   * @param zoom ズームレベル
   * @param x 出力: タイルのX座標
   * @param y 出力: タイルのY座標
   * @param pixelX 出力: タイル内のピクセルX座標
   * @param pixelY 出力: タイル内のピクセルY座標
   */
  void latLonToTile(double lat, double lon, int zoom, int &x, int &y, int &pixelX, int &pixelY);

  /**
   * @brief タイル座標から緯度経度に変換
   * @param x タイルのX座標
   * @param y タイルのY座標
   * @param zoom ズームレベル
   * @param pixelX タイル内のピクセルX座標
   * @param pixelY タイル内のピクセルY座標
   * @param lat 出力: 緯度
   * @param lon 出力: 経度
   */
  void tileToLatLon(int x, int y, int zoom, int pixelX, int pixelY, double &lat, double &lon);

  /**
   * @brief マップタイルを読み込む
   * @param zoom ズームレベル
   * @param x タイルのX座標
   * @param y タイルのY座標
   * @return 読み込み成功でtrue
   */
  bool loadMapTile(int zoom, int x, int y);

  /**
   * @brief マップタイルを描画
   * @param tile マップタイル
   * @param x 描画位置X
   * @param y 描画位置Y
   */
  void drawMapTile(const MapTile &tile, int x, int y);

  /**
   * @brief 方位記号を描画
   * @param x 描画位置X
   * @param y 描画位置Y
   * @param size サイズ
   */
  void drawCompass(int x, int y, int size);

  /**
   * @brief スケールバーを描画
   * @param x 描画位置X
   * @param y 描画位置Y
   */
  void drawScaleBar(int x, int y);
};

extern DisplayManager displayManager;
