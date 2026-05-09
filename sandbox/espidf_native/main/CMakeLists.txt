idf_component_register(
    SRCS "src/main.cpp" "src/native_app.cpp" "src/native_lcd.cpp"
         "src/native_renderer.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd esp_timer
)
