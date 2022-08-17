cmake_minimum_required(VERSION 3.14)
project(asaloader VERSION 3.0.0 LANGUAGES CXX)

# ==== add compiler setting flag ===================================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================================================= #
# ==== add QT code generator ================================#

# set(CMAKE_AUTOUIC ON) // .ui file
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON) // .qrc file 

# ================================================================= #

# ==== find used QT package ======================================= #
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core LinguistTools SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core LinguistTools SerialPort)

qt_standard_project_setup()
# ================================================================= #

add_subdirectory(asaloader)