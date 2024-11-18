idf_component_register(SRCS "src/main.c" "src/timer.c" "src/lcd.c" "src/ui.c" "src/ir.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_lcd lvgl)