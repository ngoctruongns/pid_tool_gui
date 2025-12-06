cmake_minimum_required(VERSION 3.5)
project(pid_tool_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable automoc so Q_OBJECT classes are moc'ed automatically
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets SerialPort Charts REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SRC
    src/*.cpp
)
file(GLOB HDR
    include/*.h
)

add_executable(pid_tool_gui ${SRC} ${HDR})

target_link_libraries(pid_tool_gui
    Qt5::Widgets
    Qt5::SerialPort
    Qt5::Charts
)
