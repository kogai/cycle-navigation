
/**
 * LORA and SD card share the same SPI.
 * This example tests the time-sharing multiplexing function of LORA and SD card
 * 
 * It is necessary to plug in the memory card for testing, 
 * otherwise the SD card will restart if it cannot be detected
*/

#include <RadioLib.h>
#include "utilities.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

SX1262 radio = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_BUSY);

int transmissionState = RADIOLIB_ERR_NONE;

void lora_init(void)
{
    // initialize SX1262 with default settings
    Serial.println(F("[SX1262] Initializing ... "));
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("[SX1262] init success!"));
    }
    else
    {
        Serial.print(F("[SX1262] init failed, code "));
        Serial.println(state);
        while (true)
            ;
    }

    // set the function that will be called
    // when packet transmission is finished
    radio.setPacketSentAction(setFlag);

    if (radio.setFrequency(850.0) == RADIOLIB_ERR_INVALID_FREQUENCY)
    {
        Serial.println(F("Selected frequency is invalid for this module!"));
        while (true)
            ;
    }

    // set bandwidth to 250 kHz
    if (radio.setBandwidth(125.0) == RADIOLIB_ERR_INVALID_BANDWIDTH)
    {
        Serial.println(F("Selected bandwidth is invalid for this module!"));
        while (true)
            ;
    }

    // set spreading factor to 10
    if (radio.setSpreadingFactor(10) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
    {
        Serial.println(F("Selected spreading factor is invalid for this module!"));
        while (true)
            ;
    }

    // set coding rate to 6
    if (radio.setCodingRate(6) == RADIOLIB_ERR_INVALID_CODING_RATE)
    {
        Serial.println(F("Selected coding rate is invalid for this module!"));
        while (true)
            ;
    }

    // set LoRa sync word to 0xAB
    if (radio.setSyncWord(0xAB) != RADIOLIB_ERR_NONE)
    {
        Serial.println(F("Unable to set sync word!"));
        while (true)
            ;
    }

    // set output power to 10 dBm (accepted range is -17 - 22 dBm)
    if (radio.setOutputPower(10) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
    {
        Serial.println(F("Selected output power is invalid for this module!"));
        while (true)
            ;
    }

    // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
    // NOTE: set value to 0 to disable overcurrent protection
    if (radio.setCurrentLimit(140) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT)
    {
        Serial.println(F("Selected current limit is invalid for this module!"));
        while (true)
            ;
    }

    // set LoRa preamble length to 15 symbols (accepted range is 0 - 65535)
    if (radio.setPreambleLength(15) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH)
    {
        Serial.println(F("Selected preamble length is invalid for this module!"));
        while (true)
            ;
    }

    // disable CRC
    if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION)
    {
        Serial.println(F("Selected CRC is invalid for this module!"));
        while (true)
            ;
    }

    // Some SX126x modules have TCXO (temperature compensated crystal
    // oscillator). To configure TCXO reference voltage,
    // the following method can be used.
    if (radio.setTCXO(2.4) == RADIOLIB_ERR_INVALID_TCXO_VOLTAGE)
    {
        Serial.println(F("Selected TCXO voltage is invalid for this module!"));
        while (true)
            ;
    }

    // Some SX126x modules use DIO2 as RF switch. To enable
    // this feature, the following method can be used.
    // NOTE: As long as DIO2 is configured to control RF switch,
    //       it can't be used as interrupt pin!
    if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE)
    {
        Serial.println(F("Failed to set DIO2 as RF switch!"));
        while (true)
            ;
    }

    Serial.println(F("[SX1262] All settings succesfully changed!"));

    // start transmitting the first packet
    Serial.println(F("[SX1262] Sending first packet ... "));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    transmissionState = radio.startTransmit("Hello World!");

    // you can also transmit byte array up to 256 bytes long
    /*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      state = radio.startTransmit(byteArr, 8);
    */
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void sd_cart_init(void)
{
    if (SD.begin(SD_CS))
    {
        Serial.println("[SD card] init success");
    }
    else 
    {
        Serial.println("[SD card] Card Mount Failed");
        while (true)
            ;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("[SD card] No SD card attached");
        while (true)
            ;
    }

    Serial.print("[SD card] SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[SD card] Size: %lluMB\n", cardSize);

}

void setup()
{
    // lora and sd use the same spi, in order to avoid mutual influence;
    // before powering on, all CS signals should be pulled high and in an unselected state;
    pinMode(LORA_CS, OUTPUT);
    digitalWrite(LORA_CS, HIGH);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    // Turn on the screen backlight
    pinMode(BL_EN, OUTPUT);
    digitalWrite(BL_EN, HIGH);

    Serial.begin(115200);

    SPI.begin(BOARD_SPI_SCLK, BOARD_SPI_MISO, BOARD_SPI_MOSI);

    Serial.println(F("------------------------------------------"));

    lora_init();

    sd_cart_init();

}

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void)
{
    // we sent a packet, set the flag
    transmittedFlag = true;
}

// counter to keep track of transmitted packets
int count = 0;

void loop()
{
    // check if the previous transmission finished
    if (transmittedFlag)
    {
        // reset flag
        transmittedFlag = false;

        if (transmissionState == RADIOLIB_ERR_NONE)
        {
            // packet was successfully sent
            Serial.println(F("transmission finished!"));

            // NOTE: when using interrupt-driven transmit method,
            //       it is not possible to automatically measure
            //       transmission data rate using getDataRate()
        }
        else
        {
            Serial.print(F("failed, code "));
            Serial.println(transmissionState);
        }

        // clean up after transmission is finished
        // this will ensure transmitter is disabled,
        // RF switch is powered down etc.
        radio.finishTransmit();

        // wait a second before transmitting again
        delay(3000);

        // send another one
        Serial.print(F("[SX1262] Sending another packet ... "));

        // you can transmit C-string or Arduino string up to
        // 256 characters long
        String str = "Hello World! #" + String(count++);
        transmissionState = radio.startTransmit(str);

        // you can also transmit byte array up to 256 bytes long
        /*
          byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                            0x89, 0xAB, 0xCD, 0xEF};
          transmissionState = radio.startTransmit(byteArr, 8);
        */

       listDir(SD, "/", 0);
    }
    delay(1);
}
