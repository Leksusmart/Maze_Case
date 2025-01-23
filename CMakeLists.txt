cmake_minimum_required(VERSION 3.16)

project(MazeCase VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
set_source_files_properties(meta.rc PROPERTIES LANGUAGE RC)

set(PROJECT_SOURCES
        src/main.cpp
        src/WelcomeWindow.cpp
        src/WelcomeWindow.h
        src/WelcomeWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MazeCase
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/resouses.qrc
        src/MazeWindow.h src/MazeWindow.cpp src/MazeWindow.ui
        src/ItemInfoDialog.h src/ItemInfoDialog.cpp src/ItemInfoDialog.ui
        src/CaseOpenDialog.h src/CaseOpenDialog.cpp src/CaseOpenDialog.ui
        src/meta.rc

    )
else()
    if(ANDROID)
        add_library(MazeCase SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(MazeCase
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(MazeCase PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Network)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MazeCase)
endif()
set_target_properties(MazeCase PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MazeCase
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MazeCase)
endif()
