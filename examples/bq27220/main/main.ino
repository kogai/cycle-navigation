
#include "bq27220.h"
#include "Wire.h"

#define BOARD_I2C_SDA  39
#define BOARD_I2C_SCL  40

BQ27220 bq;
BQ27220BatteryStatus batt;

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    // necessary init
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    
    bq.init();
}

void loop() 
{
    Serial.printf("-------------------------------------------\n");
    bq.getBatteryStatus(&batt);
    Serial.printf("Status = %x\n", batt.full);
    Serial.printf("Status = %s\n", bq.getIsCharging() ? "Charging" : "Discharging");
    Serial.printf("Volt = %d mV\n", bq.getVoltage());
    Serial.printf("Curr = %d mA\n", bq.getCurrent());
    Serial.printf("Temp = %.2f K\n", (float)(bq.getTemperature() / 10.0));
    Serial.printf("full cap= %d mAh\n", bq.getFullChargeCapacity());
    Serial.printf("remain cap = %d mAh\n", bq.getRemainingCapacity());
    Serial.printf("state of charge = %d\n", bq.getStateOfCharge());
    Serial.printf("state of health = %d\n", bq.getStateOfHealth());
    delay(1000);
}


