idf_component_register(
    SRCS "src/app.cpp" "src/app_fsm.cpp"
    INCLUDE_DIRS "include"
    REQUIRES arduino-esp32 board diagnostics display hx711 pn532 m5unified
)
