#include "gps_manager.h"
#include "power_manager.h"

// グローバルインスタンス
GPSManager gpsManager;

GPSManager::GPSManager() : lastUpdateTime(0),
                           lastGPSRead(0),
                           lowPowerMode(false),
                           locationValid(false),
                           latitude(0.0),
                           longitude(0.0),
                           altitude(0.0),
                           speed(0.0),
                           course(0.0),
                           satellites(0),
                           signalQuality(0)
{
}

bool GPSManager::init()
{
  // GPSシリアルポートの初期化
  SerialGPS.begin(38400, SERIAL_8N1, BOARD_GPS_RXD, BOARD_GPS_TXD);

  // GPSモジュールの初期化
  return setupGPS();
}

bool GPSManager::update()
{
  unsigned long currentMillis = millis();
  bool updated = false;

  // 低電力モードの場合、一定間隔でのみGPSを更新
  if (lowPowerMode && (currentMillis - lastGPSRead < GPS_UPDATE_INTERVAL))
  {
    return false;
  }

  // GPSデータの読み取り
  while (SerialGPS.available())
  {
    int c = SerialGPS.read();
    if (gps.encode(c))
    {
      // 有効な位置情報が得られた場合
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        locationValid = true;

        // その他の情報も更新
        if (gps.altitude.isValid())
        {
          altitude = gps.altitude.meters();
        }

        if (gps.speed.isValid())
        {
          speed = gps.speed.kmph();
        }

        if (gps.course.isValid())
        {
          course = gps.course.deg();
        }

        if (gps.satellites.isValid())
        {
          satellites = gps.satellites.value();
        }

        // 信号品質の計算（衛星数に基づく簡易的な計算）
        if (satellites >= 10)
        {
          signalQuality = 4;
        }
        else if (satellites >= 7)
        {
          signalQuality = 3;
        }
        else if (satellites >= 5)
        {
          signalQuality = 2;
        }
        else if (satellites >= 3)
        {
          signalQuality = 1;
        }
        else
        {
          signalQuality = 0;
        }

        lastUpdateTime = currentMillis;
        updated = true;
      }
    }
  }

  lastGPSRead = currentMillis;

  // 低電力モードの場合、読み取り後にGPSをスリープ
  if (lowPowerMode && updated)
  {
    // GPSモジュールをスリープモードに
    powerManager.disableGPS();
  }

  return updated;
}

double GPSManager::getLatitude()
{
  return latitude;
}

double GPSManager::getLongitude()
{
  return longitude;
}

double GPSManager::getAltitude()
{
  return altitude;
}

double GPSManager::getSpeed()
{
  return speed;
}

double GPSManager::getCourse()
{
  return course;
}

int GPSManager::getSatellites()
{
  return satellites;
}

unsigned long GPSManager::getLastUpdateTime()
{
  return lastUpdateTime;
}

int GPSManager::getSignalQuality()
{
  return signalQuality;
}

void GPSManager::enableLowPowerMode()
{
  lowPowerMode = true;
}

void GPSManager::disableLowPowerMode()
{
  lowPowerMode = false;
  // 低電力モード解除時にGPSを有効化
  powerManager.enableGPS();
}

bool GPSManager::isLocationValid()
{
  return locationValid;
}

bool GPSManager::setupGPS()
{
  bool result = false;

  // MIA-M10Q GPSモジュールのボーレート設定
  // まず38400で試す
  SerialGPS.begin(38400, SERIAL_8N1, BOARD_GPS_RXD, BOARD_GPS_TXD);
  result = gpsRecovery();

  // 失敗したら9600で試す
  if (!result)
  {
    SerialGPS.updateBaudRate(9600);
    result = gpsRecovery();
    if (!result)
    {
      Serial.println("GPS Connect failed!");
      return false;
    }
    // 成功したら38400に戻す
    SerialGPS.updateBaudRate(38400);
  }

  // GPSモジュールの電源を有効化
  powerManager.enableGPS();

  return result;
}

bool GPSManager::gpsRecovery()
{
  uint8_t buffer[256];

  // UBX-CFG-RATE, Size 8, 'Navigation/measurement rate settings'
  uint8_t cfg_rate[] = {0xB5, 0x62, 0x06, 0x08, 0x00, 0x00, 0x0E, 0x30};
  SerialGPS.write(cfg_rate, sizeof(cfg_rate));

  // 応答を待つ
  uint16_t ubxFrameCounter = 0;
  uint16_t needRead = 0;
  uint32_t startTime = millis();

  while (millis() - startTime < 800)
  {
    while (SerialGPS.available())
    {
      int c = SerialGPS.read();
      switch (ubxFrameCounter)
      {
      case 0:
        if (c == 0xB5)
        {
          ubxFrameCounter++;
        }
        break;
      case 1:
        if (c == 0x62)
        {
          ubxFrameCounter++;
        }
        else
        {
          ubxFrameCounter = 0;
        }
        break;
      case 2:
        if (c == 0x06)
        {
          ubxFrameCounter++;
        }
        else
        {
          ubxFrameCounter = 0;
        }
        break;
      case 3:
        if (c == 0x08)
        {
          ubxFrameCounter++;
        }
        else
        {
          ubxFrameCounter = 0;
        }
        break;
      case 4:
        needRead = c;
        ubxFrameCounter++;
        break;
      case 5:
        needRead |= (c << 8);
        ubxFrameCounter++;
        break;
      case 6:
        if (needRead >= 256)
        {
          ubxFrameCounter = 0;
          break;
        }
        if (SerialGPS.readBytes(buffer, needRead) != needRead)
        {
          ubxFrameCounter = 0;
        }
        else
        {
          return true;
        }
        break;
      default:
        break;
      }
    }
  }

  return false;
}
