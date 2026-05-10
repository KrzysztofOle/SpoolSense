idf_component_register(
    SRCS "src/display.cpp"
    INCLUDE_DIRS "include"
    REQUIRES diagnostics driver esp_lcd esp_timer lvgl
)
