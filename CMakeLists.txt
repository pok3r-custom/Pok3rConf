cmake_minimum_required(VERSION 3.0)

project(Pok3rConf)

add_subdirectory(pok3rtool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Quick Qml QuickWidgets Widgets Network REQUIRED)

qt5_wrap_ui(UI_SRC
    mainwindow.ui
    editor/keycustomize.ui
)

qt5_add_resources(RC_SRC
    resource.qrc
)

set(SRC
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainworker.h
    mainworker.cpp
    editor/keycustomize.h
    editor/keycustomize.cpp
    ${UI_SRC}
    ${RC_SRC}
)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${QtQml_INCLUDE_DIRS}
)

GitDescribe("${CMAKE_CURRENT_SOURCE_DIR}" POK3RCONF_DESCRIBE)
message(STATUS "Pok3rConf: ${POK3RCONF_DESCRIBE}")
set_source_files_properties(main.cpp PROPERTIES COMPILE_DEFINITIONS _POK3RCONF_DESCRIBE="${POK3RCONF_DESCRIBE}")

if(WIN32)
    add_executable(pok3rconf WIN32 ${SRC})
else()
    add_executable(pok3rconf ${SRC})
endif()
set_property(TARGET pok3rconf PROPERTY CXX_STANDARD 11)
#qt5_use_modules(pok3rconf Quick Core)
target_link_libraries(pok3rconf pok3rlib Qt5::Widgets Qt5::Quick Qt5::Qml Qt5::QuickWidgets Qt5::Network)

# install
install(TARGETS pok3rconf DESTINATION bin)

# copy shared libraries to build dir
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
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::Quick>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::Qml>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::QuickWidgets>"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt5::Network>"
            "$<TARGET_FILE_DIR:pok3rconf>"
    )
endif()

if(MINGW)
    # Apparently Qt's mingw doesn't come with some static libraries
    get_filename_component(MINGW_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    set(MINGW_LIBS_DIR "${MINGW_DIR}/../i686-w64-mingw32/lib")
    set(MINGW_LIBGCC "${MINGW_LIBS_DIR}/libgcc_s_dw2-1.dll")
    set(MINGW_LIBSTD "${MINGW_LIBS_DIR}/libstdc++-6.dll")
    set(MINGW_LIBWPT "${MINGW_LIBS_DIR}/libwinpthread-1.dll")
    set(MINGW_LIBEAY32 "${MINGW_DIR}/../opt/bin/libeay32.dll")
    set(MINGW_SSLEAY32 "${MINGW_DIR}/../opt/bin/ssleay32.dll")

    add_custom_command(TARGET pok3rconf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_LIBGCC}"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_LIBSTD}"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_LIBWPT}"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_LIBEAY32}"
            "$<TARGET_FILE_DIR:pok3rconf>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_SSLEAY32}"
            "$<TARGET_FILE_DIR:pok3rconf>"
    )
endif()

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
