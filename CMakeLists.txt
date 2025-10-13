cmake_minimum_required(VERSION 3.16)
project(HyprSessionManager)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(hyprsessionmanager
    main.cpp
    SessionManager.cpp
    SessionManager.h
)

target_link_libraries(hyprsessionmanager PRIVATE Qt6::Widgets)
