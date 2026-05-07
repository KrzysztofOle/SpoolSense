idf_component_register(
    SRCS "src/Adafruit_BusIO_Register.cpp" "src/Adafruit_GenericDevice.cpp"
         "src/Adafruit_I2CDevice.cpp" "src/Adafruit_PN532.cpp"
         "src/Adafruit_SPIDevice.cpp" "src/pn532_reader.cpp"
    INCLUDE_DIRS "include"
    REQUIRES arduino-esp32 board
)
