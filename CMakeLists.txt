cmake_minimum_required(VERSION 3.26)

project(qtvsgviewer VERSION 1.0.0 LANGUAGES CXX)

find_package(PkgConfig)
pkg_check_modules(FMT REQUIRED fmt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Uncomment these for Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

subdirs(src)
