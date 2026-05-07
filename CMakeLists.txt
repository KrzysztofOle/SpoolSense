cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    "components/app"
    "components/board"
    "components/diagnostics"
    "components/display"
    "components/hx711"
    "components/pn532"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(spoolsense)
