cmake_minimum_required(VERSION 3.18)
project(esm LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# 1) Core library (C++ logic)
# -----------------------------
add_library(esm_core STATIC
    esm/Engine.cpp
    storage/SqliteStorage.cpp
    storage/DeviceRepository.cpp
    storage/WorkerRepository.cpp
    sqlite/sqlite3.c


    Device.cpp
    HealthyDevice.cpp
    ReserveDevice.cpp
    FaultyDeviceEx.cpp
    faultyDevice.cpp
)

set_source_files_properties(sqlite/sqlite3.c PROPERTIES LANGUAGE C)


target_include_directories(esm_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite
)

target_compile_definitions(esm_core PRIVATE
    ESM_BUILD=1
)


# -----------------------------
# 2) Python module (pybind11)
# -----------------------------
# bindings/CMakeLists.txt должен собрать модуль через pybind11_add_module(...)
add_subdirectory(bindings)

# -----------------------------
# 3) C++ tests (main.cpp)
# -----------------------------
option(ESM_BUILD_TESTS "Build C++ tests" OFF)

if (ESM_BUILD_TESTS)
    enable_testing()

    add_executable(esm_tests
        main.cpp
    )

    target_link_libraries(esm_tests PRIVATE esm_core)
    add_test(NAME esm_tests COMMAND esm_tests)
endif()
