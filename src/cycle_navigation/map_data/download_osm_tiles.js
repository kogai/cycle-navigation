/**
 * OpenStreetMapから地図データを取得するスクリプト
 * Node.jsで実行するためのスクリプト
 */

const https = require("https");
const fs = require("fs");
const path = require("path");

// OpenStreetMapのタイルサーバーURL
const OSM_TILE_URL = "https://tile.openstreetmap.org/{z}/{x}/{y}.png";

// 保存先ディレクトリ
const OUTPUT_DIR = path.join(__dirname, "tiles");

/**
 * 緯度経度からタイル座標に変換する関数
 * @param {number} lat - 緯度
 * @param {number} lon - 経度
 * @param {number} zoom - ズームレベル
 * @returns {Object} タイル座標 {x, y}
 */
function latLonToTile(lat, lon, zoom) {
  const n = Math.pow(2, zoom);
  const x = Math.floor(((lon + 180) / 360) * n);
  const y = Math.floor(
    ((1 -
      Math.log(
        Math.tan((lat * Math.PI) / 180) + 1 / Math.cos((lat * Math.PI) / 180)
      ) /
        Math.PI) /
      2) *
      n
  );
  return { x, y };
}

/**
 * 指定された範囲のタイルURLを生成する関数
 * @param {number} minLat - 最小緯度
 * @param {number} minLon - 最小経度
 * @param {number} maxLat - 最大緯度
 * @param {number} maxLon - 最大経度
 * @param {number} zoom - ズームレベル
 * @returns {Array} タイル情報の配列 [{url, x, y, z}]
 */
function generateTileUrls(minLat, minLon, maxLat, maxLon, zoom) {
  const minTile = latLonToTile(minLat, minLon, zoom);
  const maxTile = latLonToTile(maxLat, maxLon, zoom);

  const tiles = [];

  for (let x = minTile.x; x <= maxTile.x; x++) {
    for (let y = minTile.y; y <= maxTile.y; y++) {
      const url = OSM_TILE_URL.replace("{z}", zoom)
        .replace("{x}", x)
        .replace("{y}", y);
      tiles.push({ url, x, y, z: zoom });
    }
  }

  return tiles;
}

/**
 * タイルを取得してファイルに保存する関数
 * @param {Object} tile - タイル情報 {url, x, y, z}
 * @returns {Promise} 保存完了のPromise
 */
function downloadTile(tile) {
  return new Promise((resolve, reject) => {
    const fileName = `${tile.z}_${tile.x}_${tile.y}.png`;
    const filePath = path.join(OUTPUT_DIR, fileName);

    // ディレクトリが存在しない場合は作成
    if (!fs.existsSync(OUTPUT_DIR)) {
      fs.mkdirSync(OUTPUT_DIR, { recursive: true });
    }

    // ファイルが既に存在する場合はスキップ
    if (fs.existsSync(filePath)) {
      console.log(`File already exists: ${fileName}`);
      resolve();
      return;
    }

    const file = fs.createWriteStream(filePath);

    https
      .get(tile.url, (response) => {
        if (response.statusCode !== 200) {
          reject(new Error(`Failed to download tile: ${response.statusCode}`));
          return;
        }

        response.pipe(file);

        file.on("finish", () => {
          file.close();
          console.log(`Downloaded: ${fileName}`);
          resolve();
        });
      })
      .on("error", (err) => {
        fs.unlink(filePath, () => {}); // エラー時にファイルを削除
        reject(err);
      });
  });
}

/**
 * 複数のタイルを順番に取得する関数
 * @param {Array} tiles - タイル情報の配列
 * @returns {Promise} 全タイル取得完了のPromise
 */
async function downloadTiles(tiles) {
  console.log(`Downloading ${tiles.length} tiles...`);

  let completed = 0;

  for (const tile of tiles) {
    try {
      await downloadTile(tile);
      completed++;
      console.log(
        `Progress: ${completed}/${tiles.length} (${Math.round(
          (completed / tiles.length) * 100
        )}%)`
      );
    } catch (error) {
      console.error(
        `Error downloading tile ${tile.z}/${tile.x}/${tile.y}: ${error.message}`
      );
    }

    // OpenStreetMapの利用規約に従い、リクエスト間に少し遅延を入れる
    await new Promise((resolve) => setTimeout(resolve, 200));
  }

  return completed;
}

/**
 * メイン関数 - 指定された範囲の地図データを取得する
 */
async function main() {
  // 東京の緯度経度範囲
  const minLat = 35.65;
  const minLon = 139.7;
  const maxLat = 35.75;
  const maxLon = 139.8;
  const zoom = 13;

  console.log(
    `Downloading map tiles for area: (${minLat},${minLon}) to (${maxLat},${maxLon}) at zoom level ${zoom}`
  );

  // タイル情報を生成
  const tiles = generateTileUrls(minLat, minLon, maxLat, maxLon, zoom);

  // タイルをダウンロード
  const downloadedCount = await downloadTiles(tiles);

  console.log(
    `Download completed. ${downloadedCount} tiles saved to ${OUTPUT_DIR}`
  );
}

// スクリプト実行時に自動的にダウンロードを開始
main().catch((error) => {
  console.error(`Error: ${error.message}`);
  process.exit(1);
});
