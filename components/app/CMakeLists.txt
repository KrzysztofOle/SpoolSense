idf_component_register(
    SRCS "src/app.cpp" "src/app_fsm.cpp"
    INCLUDE_DIRS "include"
    REQUIRES arduino-esp32 board diagnostics display driver hx711 nvs_flash pn532
)
