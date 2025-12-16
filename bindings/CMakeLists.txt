# bindings/CMakeLists.txt

cmake_minimum_required(VERSION 3.18)

# Заставляем pybind11 использовать современный FindPython
set(PYBIND11_FINDPYTHON ON)

# Явно находим Python (interpreter + dev libs)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Теперь подключаем pybind11
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(equipment_status_monitoring MODULE
    bindings.cpp
)

target_link_libraries(equipment_status_monitoring PRIVATE esm_core)
target_include_directories(equipment_status_monitoring PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

install(TARGETS equipment_status_monitoring
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
    ARCHIVE DESTINATION .
)