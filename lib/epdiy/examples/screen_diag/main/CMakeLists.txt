idf_component_register(SRCS "screen_diag.c" "epd.c" "commands.c" "commands/system.c"
        "commands/screen.c" "commands/graphics.c" "commands/tests.c"
                    INCLUDE_DIRS "." "res/fonts"
                    REQUIRES epdiy console nvs_flash fatfs esp_app_format)
