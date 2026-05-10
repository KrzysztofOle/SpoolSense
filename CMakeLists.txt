cmake_minimum_required(VERSION 3.16)

add_compile_definitions(M5GFX_BOARD=board_M5Stack)
add_compile_definitions(ARDUINO_M5STACK_CORE_ESP32)

set(EXTRA_COMPONENT_DIRS
    "components/app"
    "components/board"
    "components/diagnostics"
    "components/display"
    "components/hx711"
    "components/pn532"
    "components/spool_tag"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(spoolsense)
