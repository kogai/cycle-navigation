


<h1 align = "center">🏆T5_E_Paper_S3_Pro🏆</h1>

![Build Status](https://github.com/Xinyuan-LilyGO/T5S3-4.7-e-paper-PRO/actions/workflows/platformio.yml/badge.svg?event=push)

<p> 
  <a href="https://platformio.org/"> <img src="./images/PlatformIO_badge.png" height="20px"> </a>
  <a href="https://www.arduino.cc/en/software"> <img src="./images/Arduino_badge.png" height="20px"></a>
  <a href="https://lilygo.cc/products/t5-e-paper-s3-pro"> <img src="https://img.shields.io/badge/Liiygo-T5S3_E_Paper_S3_PRO-blue" height="20px"></a>
  <a href=""> <img src="https://img.shields.io/badge/language-c++-brightgreen" height="20px"></a>
</p>

<!-- * [切换到中文版](./README_CN.md) -->

## :one:Product 🎁

|     Product      | [T5 E-Paper S3 Pro](https://lilygo.cc/products/t5-e-paper-s3-pro) |
| :--------------: | :---------------------------------------------------------------: |
|       MCU        |                         ESP32-S3-WROOM-1                          |
|  Flash / PSRAM   |                             16M / 8M                              |
|       Lora       |                              SX1262                               |
|      Touch       |                           GT911 (0x5D)                            |
|    Driver IC     |             ED047TC1 (4.7 inches, 960x540 , 16 gray)              |
| Battery Capacity |                              1500mAh                              |
|   Battery Chip   |                  BQ25896 (0x6B), BQ27220 (0x55)                   |
|       RTC        |                          PCF85063 (0x51)                          |

## :two:Quick Start 🎁

LORA and SD use the same spi, in order to avoid mutual influence; before powering on, all CS signals should be pulled high and in an unselected state;

~~~arduino
void setup()
{
    pinMode(LORA_CS, OUTPUT);
    digitalWrite(LORA_CS, HIGH);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    ...
}
~~~

After power-on, click to enter the WIFI interface, configure the network, you can use;

## :three:Use PlatformIO

The project uses PlatformIO development, as long as clone code, compile and download can run;

If you are the first to use PlatformIO, you can try the following steps to install PlatformIO:

1. Install [Visual Studio Code](https://code.visualstudio.com/) and [Python](https://www.python.org/), and clone or download the project;
2. Search for the `PlatformIO` plugin in the `VisualStudioCode` extension and install it;
3. After the installation is complete, you need to restart `VisualStudioCode`
4. After opening this project, PlatformIO will automatically download the required tripartite libraries and dependencies, the first time this process is relatively long, please wait patiently;
5. After all the dependencies are installed, you can open the `platformio.ini` configuration file, uncomment in `example` to select a routine, and then press `ctrl+s` to save the `.ini` configuration file;
6. Click :ballot_box_with_check: under VScode to compile the project, then plug in USB and select COM under VScode;
7. Finally, click the :arrow_right:  button to download the program to Flash;


<!-- 
## 🔴🟡🟢 Running effect drawing
![](./images/epd_clock.png)
![](./images/epd_lora.png)
![](./images/esp_sd.png)
![](./images/esp_test.png)
![](./images/esp_wifi.png) -->