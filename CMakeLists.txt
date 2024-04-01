cmake_minimum_required(VERSION 3.5)

project(a_star_viewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

add_executable(a_star_viewer
    ${PROJECT_SOURCES}
    astarview.h astarview.cpp astarview.ui
    model/astarmodel.h model/astarmodel.cpp   
)

target_link_libraries(a_star_viewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Pulled git submodule
add_subdirectory(a_star)
target_link_libraries(a_star_viewer PRIVATE a_star_lib)
