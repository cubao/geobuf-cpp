cmake_minimum_required(VERSION 3.15...3.27)

project(
    ${SKBUILD_PROJECT_NAME}
    VERSION ${SKBUILD_PROJECT_VERSION}
    LANGUAGES CXX)

# set(CMAKE_BUILD_TYPE "Debug")
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "" FORCE)
    message(STATUS "Set build type to default: ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS "Your build type: ${CMAKE_BUILD_TYPE}")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pragma-once-outside-header")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
add_definitions(-DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/headers/include)
include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/geobuf)

option(BUILD_SHARED_LIBS "Build shared library." OFF)
set(HEADER_ROOT "${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}")
file(GLOB_RECURSE HEADERS src/**/*.hpp)
file(GLOB_RECURSE SOURCES src/**/*.cpp)
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET "hidden")
install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_NAME})

add_executable(pbf_decoder src/geobuf/pbf_decoder.cpp)
target_compile_definitions(pbf_decoder PUBLIC -DPBF_DECODER_ENABLE_MAIN)
install(TARGETS pbf_decoder RUNTIME DESTINATION ${PROJECT_NAME})

option(BUILD_EXAMPLES "Build examples." ON)
if(BUILD_EXAMPLES)
    file(GLOB SRCS examples/*.cpp)
    foreach(src ${SRCS})
        string(
            REGEX
            REPLACE "(^.*/|.cpp$)"
                    ""
                    exe
                    ${src})
        add_executable(${exe} ${src})
        target_link_libraries(${exe} ${PROJECT_NAME})
        install(TARGETS ${exe} RUNTIME DESTINATION ${PROJECT_NAME})
    endforeach(src)
endif()

option(BUILD_TESTS "Build tests." ON)
if(BUILD_TESTS)
    file(GLOB SRCS tests/test_*.cpp)
    foreach(src ${SRCS})
        string(
            REGEX
            REPLACE "(^.*/|.cpp$)"
                    ""
                    exe
                    ${src})
        add_executable(${exe} ${src})
        target_link_libraries(${exe} ${PROJECT_NAME})
        install(TARGETS ${exe} RUNTIME DESTINATION ${PROJECT_NAME})
    endforeach(src)
endif()
add_custom_target(test_all COMMAND make -C .. test_all)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

set(PYBIND_SRCS src/main.cpp src/pybind11_rapidjson.cpp src/pybind11_geojson.cpp)
python_add_library(
    _core
    MODULE
    ${PYBIND_SRCS}
    WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers ${PROJECT_NAME})
target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})
install(TARGETS _core DESTINATION ${PROJECT_NAME})
