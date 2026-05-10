idf_component_register(
    SRCS "src/board.cpp" "src/buttons.cpp"
    INCLUDE_DIRS "include"
    REQUIRES arduino-esp32 diagnostics
)
