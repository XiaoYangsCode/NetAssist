cmake_minimum_required(VERSION 3.5)

project(NetAssist VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialBus)

set(SRC_DIR source)
set(HEAD_DIR include)
set(UI_DIR ui)
file(GLOB SRC_FILES "${SRC_DIR}/*.cpp" "${UI_DIR}/*.cpp")
file(GLOB HEAD_FILES "${HEAD_DIR}/*.h" "${UI_DIR}/*.h")
file(GLOB UI_FILES "${UI_DIR}/*.ui")
source_group("Ui" FILES ${UI_FILES})

set(PROJECT_SOURCES
        main.cpp
        ${SRC_FILES}
        ${HEAD_FILES}
        ${UI_FILES}
    )


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()
include_directories("${UI_DIR}" "${HEAD_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::SerialBus)
