cmake_minimum_required(VERSION 3.0)

project(Pok3rConf)

add_subdirectory(pok3rtool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

qt5_wrap_ui(UI_SRC
    mainwindow.ui
)

set(SRC
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainworker.h
    mainworker.cpp
    ${UI_SRC}
)

if(WIN32)
    add_executable(pok3rconf WIN32 ${SRC})
else()
    add_executable(pok3rconf ${SRC})
endif()
target_link_libraries(pok3rconf chaos pok3rlib Qt5::Widgets)

# Copy shared libraries to build dir
add_custom_command(TARGET pok3rconf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:chaos>"
        "$<TARGET_FILE_DIR:pok3rconf>"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:pok3rlib>"
        "$<TARGET_FILE_DIR:pok3rconf>"
)

if(WIN32)
    add_custom_command(TARGET pok3rconf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:Qt5::Core>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:Qt5::Gui>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:Qt5::Widgets>"
            "$<TARGET_FILE_DIR:pok3rconf>"
    )
endif()

if(MINGW)
    # Apparently Qt's mingw doesn't come with some static libraries
    get_filename_component(MINGW_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    add_custom_command(TARGET pok3rconf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${MINGW_DIR}/../i686-w64-mingw32/lib/libgcc_s_dw2-1.dll
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy
            ${MINGW_DIR}/../i686-w64-mingw32/lib/libstdc++-6.dll
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy
            ${MINGW_DIR}/../i686-w64-mingw32/lib/libwinpthread-1.dll
            "$<TARGET_FILE_DIR:pok3rconf>"
    )
endif()
