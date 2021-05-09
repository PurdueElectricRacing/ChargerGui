cmake_minimum_required(VERSION 3.5)

project(ChargerGui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(STUB_CAN)

set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DesktopCAN_API)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

if (WIN32)
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ")
endif()


find_package(QT NAMES Qt5 COMPONENTS Widgets core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        chargergui.cpp
        chargergui.h
        chargergui.ui
        ${SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 5)
    qt_add_executable(ChargerGui
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(ChargerGui SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ChargerGui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ChargerGui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
