cmake_minimum_required(VERSION 3.16)
project(PythonLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

set(SOURCES
    src/main.cpp
    src/appitem.cpp
    src/processrunner.cpp
    src/addappdialog.cpp
    src/logviewer.cpp
    src/mainwindow.cpp
)

set(HEADERS
    src/appitem.h
    src/processrunner.h
    src/addappdialog.h
    src/logviewer.h
    src/mainwindow.h
    src/stylehelper.h
)

add_executable(PythonLauncher ${SOURCES} ${HEADERS})

target_link_libraries(PythonLauncher PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)
