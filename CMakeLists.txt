cmake_minimum_required(VERSION 3.22)
project(esm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Библиотека esm с основным C++кодом


find_library(SQLITE3_LIBRARY sqlite3)

add_library(esm STATIC
    esm/Engine.cpp
    storage/SqliteStorage.cpp
    storage/DeviceRepository.cpp
    storage/WorkerRepository.cpp

    Device.cpp
    HealthyDevice.cpp
    ReserveDevice.cpp
    FaultyDeviceEx.cpp
    faultyDevice.cpp
)

target_include_directories(esm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(esm PRIVATE
    ESM_BUILD=1
)

if (SQLITE3_LIBRARY)
    target_link_libraries(esm PRIVATE ${SQLITE3_LIBRARY})
else()
    target_link_libraries(esm PRIVATE sqlite3)
endif()


# Модуль расширения для Python esm_py


find_package(Python3 REQUIRED COMPONENTS Interpreter Development)



set(BoostPythonComponent "python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
find_package(Boost REQUIRED COMPONENTS ${BoostPythonComponent})

add_library(esm_py MODULE
    bindings/py_esm.cpp
)

target_include_directories(esm_py PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(esm_py
    PRIVATE
        esm
        Boost::${BoostPythonComponent}
        Python3::Python
)


set_target_properties(esm_py PROPERTIES
    PREFIX ""
    OUTPUT_NAME "esm_py"
)

# C++ тесты

enable_testing()

add_executable(esm_tests
    main.cpp
)

target_link_libraries(esm_tests PRIVATE esm)

add_test(NAME esm_tests COMMAND esm_tests)
