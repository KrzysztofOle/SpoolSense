idf_component_register(
    SRCS "src/main.cpp" "src/native_app.cpp" "src/native_input.cpp"
         "src/native_lcd.cpp" "src/native_renderer.cpp"
         "src/native_screens.cpp" "src/native_lvgl_ui.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd esp_timer lvgl
)
