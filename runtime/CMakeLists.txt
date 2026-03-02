cmake_minimum_required(VERSION 3.12)

project(net_mix_test)

set(EXECUTABLE_NAME ${PROJECT_NAME}.out)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SRC ./*.cpp
    ../_HLS_CODE/fpga_top.cpp
    ../_HLS_CODE/gpool_cache.cpp
    ../_HLS_CODE/image_cache.cpp
    ../_HLS_CODE/memory_controller.cpp
    ../_HLS_CODE/netconfig.cpp
    ../_HLS_CODE/network.cpp
    ../_HLS_CODE/output_cache.cpp
    ../_HLS_CODE/processing_element.cpp
    ../_HLS_CODE/weights_cache.cpp
)

add_executable(${EXECUTABLE_NAME} 
    ${SRC}
)

target_include_directories(${EXECUTABLE_NAME}
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/../_HLS_CODE/
    ${CMAKE_CURRENT_SOURCE_DIR}/../_HLS_CODE/vivado_include
    ${CMAKE_CURRENT_SOURCE_DIR}/../_HLS_CODE/vivado_include/etc/
)

add_definitions(-DEBUG)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION /home/alinx/Workspace/zynqnet/runtime/)
