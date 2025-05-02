/**
 * 描画サイクル管理モジュール実装
 */

#include "world.h"
#include "firasans_12.h"
#include <esp_sleep.h>

World::World(Battery &battery, Display &display)
    : battery(battery), display(display)
{
}

void World::init(TwoWire &wire, int sda, int scl)
{
    battery.init(wire, sda, scl);
    display.init();
}

void World::start()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason != ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        Serial.println("デバイスがディープスリープから復帰しました");
        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("外部割り込み(RTC_IO)によって復帰しました");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("タイマーによって復帰しました");
            break;
        default:
            Serial.println("その他の理由で復帰しました");
            break;
        }
        gpio_deep_sleep_hold_dis();
    }
    else
    {
        Serial.println("通常起動しました");
    }
    render();
}

void World::enterDeepSleep()
{
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_UP_PIN, 1);
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION);
    gpio_deep_sleep_hold_en();
    esp_deep_sleep_start();
}

void World::render()
{
    display.displayCenteredText("Cycle Navigation", &FiraSans_12);
    display.updateScreen();
}
