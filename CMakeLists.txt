CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(Pok3rConf)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)

FIND_PACKAGE(Qt5 COMPONENTS Core Widgets REQUIRED)

qt5_wrap_ui(UI_SRC
    mainwindow.ui
)

SET(SRC
    main.cpp
    mainwindow.h
    mainwindow.cpp
    ${UI_SRC}
)

ADD_EXECUTABLE(pok3rconf ${SRC})
TARGET_LINK_LIBRARIES(pok3rconf Qt5::Widgets)

