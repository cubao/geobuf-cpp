cmake_minimum_required(VERSION 3.5)
project(geobuf)

# set(CMAKE_BUILD_TYPE Debug)
set(CUBAO_INCLUDE ${PROJECT_SOURCE_DIR}/headers/include)
include(${CUBAO_INCLUDE}/utils.cmake)
activate_common_configuration()
auto_build_type_and_compile_flags()
configure_output_directories()
configure_version_h()

include_directories(
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/headers/include
    ${PROJECT_SOURCE_DIR}/src)

option(BUILD_SHARED_LIBS "Build shared library." OFF)
set(HEADER_ROOT "${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}")
file(GLOB_RECURSE HEADERS src/**/*.hpp)
file(GLOB_RECURSE SOURCES src/**/*.cpp)
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
print_all_linked_libraries(${PROJECT_NAME})
install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
foreach(file ${HEADERS})
  get_filename_component(dir ${file} DIRECTORY)
  string(REGEX REPLACE "${HEADER_ROOT}" "" dir ${dir})
  install(FILES ${file} DESTINATION "include/${PROJECT_NAME}/${dir}")
endforeach()

add_executable(pbf_decoder src/geobuf/pbf_decoder.cpp)
target_compile_definitions(pbf_decoder PUBLIC -DPBF_DECODER_ENABLE_MAIN)

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
    endforeach(src)
endif()
add_custom_target(test_all COMMAND make -C .. test_all)

# print_include_directories()

# print_all_variables()
