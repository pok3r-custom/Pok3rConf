cmake_minimum_required(VERSION 3.0)

project(Pok3rConf)

add_subdirectory(pok3rtool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Quick Qml QuickWidgets Widgets REQUIRED)

qt5_wrap_ui(UI_SRC
    mainwindow.ui
    keycustomize.ui
)

qt5_add_resources(RC_SRC
    qml.qrc
)

set(SRC
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainworker.h
    mainworker.cpp
    keymapbutton.h
    keymapbutton.cpp
    keymap.h
    keymap.cpp
    keycustomize.h
    keycustomize.cpp
    ${UI_SRC}
    ${RC_SRC}
)

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS})

if(WIN32)
    add_executable(pok3rconf WIN32 ${SRC})
else()
    add_executable(pok3rconf ${SRC})
endif()
#qt5_use_modules(pok3rconf Quick Core)
target_link_libraries(pok3rconf chaos-shared pok3rlib Qt5::Widgets Qt5::Quick Qt5::Qml Qt5::QuickWidgets)

# Copy shared libraries to build dir
add_custom_command(TARGET pok3rconf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:chaos-shared>"
        "$<TARGET_FILE_DIR:pok3rconf>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:rawhid>"
        "$<TARGET_FILE_DIR:pok3rconf>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:pok3rlib>"
        "$<TARGET_FILE_DIR:pok3rconf>"
)

if(WIN32)
    add_custom_command(TARGET pok3rconf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::Core>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::Gui>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::Widgets>"
            "$<TARGET_FILE_DIR:pok3rconf>"
    )
endif()

if(MINGW)
    # Apparently Qt's mingw doesn't come with some static libraries
    get_filename_component(MINGW_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    add_custom_command(TARGET pok3rconf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MINGW_DIR}/../i686-w64-mingw32/lib/libgcc_s_dw2-1.dll
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MINGW_DIR}/../i686-w64-mingw32/lib/libstdc++-6.dll
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MINGW_DIR}/../i686-w64-mingw32/lib/libwinpthread-1.dll
            "$<TARGET_FILE_DIR:pok3rconf>"
    )
endif()
