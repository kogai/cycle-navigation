/*
This example shows how to use Preferences (nvs) to store a
structure.  Note that the maximum size of a putBytes is 496K
or 97% of the nvs partition size.  nvs has signifcant overhead,
so should not be used for data that will change often.
*/
#include <Preferences.h>
Preferences prefs;

typedef enum
{
    NVS_I8,
    NVS_U8,
    NVS_I16,
    NVS_U16,
    NVS_I32,
    NVS_U32,
    NVS_I64,
    NVS_U64,
    NVS_FLOAT,
    NVS_STR,
    NVS_BLOB,
    NVS_INVALID
} NVSType;

typedef struct
{
    int id;
    int type;
    const char *key;
    union data
    {
        uint8_t u8;
        float ff;
        const char *tp;
    } data;
} nvs_param;

/* clang-format off */
nvs_param nvs_default[] = {
    {.id = 0, .type = NVS_U8,       .key =  "blacklight",   .data = {.u8 = 99}},
    {.id = 1, .type = NVS_FLOAT,    .key =  "eps_vcom",     .data = {.ff = 1.56}},
    {.id = 2, .type = NVS_STR,      .key =  "wifi_id",      .data = {.tp = "dgx_test_wifi12"} },
};
/* clang-format on */

void setup()
{
    Serial.begin(115200);
    prefs.begin("schedule"); // use "schedule" namespace

    // bool tpInit = prefs.isKey("nvsInit"); // Test for the existence
                                          // of the "already initialized" key.
    bool tpInit = false;

    for(int i = 0; i < sizeof(nvs_default)/sizeof(nvs_default[0]); i++)
    {
        switch (nvs_default[i].type)
        {
        case NVS_U8: prefs.putUChar(nvs_default[i].key, nvs_default[i].data.u8);
            break;
        case NVS_FLOAT: prefs.putFloat(nvs_default[i].key, nvs_default[i].data.ff);
            break;
        case NVS_STR: prefs.putString(nvs_default[i].key, nvs_default[i].data.tp);
            break;
        default:
            break;
        }
    }

    char buf[15];

    int currentBrightness = prefs.getUChar("blacklight"); //
    float tChannel = prefs.getFloat("eps_vcom");        //  The LHS variables were defined
    String tChanMax = prefs.getString("wifi_id");          //   earlier in the sketch.

    Serial.printf("blacklight = %d\n", currentBrightness);
    Serial.printf("eps_vcom = %f\n", tChannel);
    Serial.printf("wifi_id = %s\n", tChanMax.c_str());

    prefs.end();

    ///
    // delay(100);
    prefs.begin("schedule");
    prefs.putUChar(nvs_default[0].key, 98);
    currentBrightness = prefs.getUChar("blacklight"); //
    Serial.printf("blacklight = %d\n", currentBrightness);

    prefs.end();
}

void loop()
{
    delay(10000);
}
