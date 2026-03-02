cmake_minimum_required(VERSION 3.12)

project(net_test)

set(EXECUTABLE_NAME ${PROJECT_NAME}.out)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCE_FILES ./*.cpp)

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/vivado_include
        ${CMAKE_CURRENT_SOURCE_DIR}/vivado_include/etc/
)

add_definitions(-DEBUG)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION /home/alinx/Workspace/zynqnet/_HLS_CODE/)
