idf_component_register(
    SRCS "src/app.cpp"
    INCLUDE_DIRS "include"
    REQUIRES arduino board diagnostics display hx711 pn532
)
