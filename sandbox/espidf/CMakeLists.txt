cmake_minimum_required(VERSION 3.16)

add_compile_definitions(ARDUINO_M5STACK_CORE_ESP32)
add_compile_definitions(M5GFX_BOARD=board_M5Stack)
add_compile_definitions(ESP_PARTITION_SUBTYPE_DATA_LITTLEFS=0x83)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(sandbox_espidf_halo)
