cmake_minimum_required(VERSION 3.5)
project(pid_tool_gui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable automoc so Q_OBJECT classes are moc'ed automatically
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets SerialPort Charts REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/serial_comm
)

file(GLOB SRC
    src/*.cpp
)
file(GLOB HDR
    include/*.h
)
file(GLOB C_SRC
    lib/serial_comm/*.c
)

add_executable(pid_tool_gui ${SRC} ${HDR} ${C_SRC})

target_link_libraries(pid_tool_gui
    Qt5::Widgets
    Qt5::SerialPort
    Qt5::Charts
)
