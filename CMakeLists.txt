cmake_minimum_required(VERSION 3.26)

project(GnotePad VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GNOTE_ENABLE_IPO "Enable interprocedural optimization" ON)
option(GNOTE_ENABLE_WARNINGS "Enable extra compiler warnings" ON)
option(GNOTE_ENABLE_CLANG_TIDY "Run clang-tidy during builds" OFF)

function(gnote_apply_default_warnings target_name)
    if(NOT GNOTE_ENABLE_WARNINGS)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wimplicit-fallthrough
            -Werror=return-type
        )
    elseif(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /EHsc)
    endif()
endfunction()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    add_compile_options($<$<CONFIG:Debug>:-Wall>)
endif()

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(GNOTE_ENABLE_CLANG_TIDY)
    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR "GNOTE_ENABLE_CLANG_TIDY is ON but clang-tidy was not found.")
    endif()
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXE}
        --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        -p ${CMAKE_BINARY_DIR}
    )
endif()

if(GNOTE_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(FetchContent)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Configuring GnotePad with clang: ${CMAKE_CXX_COMPILER}")
else()
    message(WARNING "GnotePad is validated primarily with clang; proceeding with ${CMAKE_CXX_COMPILER_ID}.")
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg SvgWidgets Test)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)

FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_library(spdlog_header_only INTERFACE)
    target_include_directories(spdlog_header_only INTERFACE ${spdlog_SOURCE_DIR}/include)
    target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_USE_STD_FORMAT=1)
    add_library(spdlog::spdlog_header_only ALIAS spdlog_header_only)
endif()

set(GNOTE_SOURCES
    src/gnotepad.cpp
    src/app/Application.cpp
    src/ui/MainWindow.cpp
    src/ui/TextEditor.cpp
)

set(GNOTE_HEADERS
    src/app/Application.h
    src/ui/MainWindow.h
    src/ui/TextEditor.h
)

qt_add_resources(GNOTE_RESOURCES resources/gnotepad.qrc)

if(CLANG_TIDY_EXE)
    add_custom_target(run-clang-tidy
        COMMAND ${CLANG_TIDY_EXE}
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            -p ${CMAKE_BINARY_DIR}
            ${GNOTE_SOURCES}
            ${GNOTE_HEADERS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on project sources"
    )
endif()

if(CLANG_FORMAT_EXE)
    add_custom_target(run-clang-format
        COMMAND ${CLANG_FORMAT_EXE} -i
            ${GNOTE_SOURCES}
            ${GNOTE_HEADERS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Applying clang-format to project sources"
    )
endif()

add_executable(GnotePad
    ${GNOTE_SOURCES}
    ${GNOTE_HEADERS}
    ${GNOTE_RESOURCES}
)

target_compile_features(GnotePad PRIVATE cxx_std_23)

gnote_apply_default_warnings(GnotePad)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(GnotePad PRIVATE
        $<$<CONFIG:Debug>:-glldb>
        $<$<CONFIG:RelWithDebInfo>:-glldb>
    )
endif()

if(WIN32)
    target_compile_definitions(GnotePad PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

set_target_properties(GnotePad PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_include_directories(GnotePad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(GnotePad PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::SvgWidgets
    spdlog::spdlog_header_only
)

target_compile_definitions(GnotePad PRIVATE GNOTE_VERSION="${PROJECT_VERSION}")

install(TARGETS GnotePad
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(UNIX AND NOT APPLE)
    set(GNOTE_DESKTOP_FILE ${CMAKE_CURRENT_BINARY_DIR}/gnotepad.desktop)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/gnotepad.desktop.in ${GNOTE_DESKTOP_FILE} @ONLY)
    install(FILES ${GNOTE_DESKTOP_FILE} DESTINATION share/applications)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/gnotepad-icon.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME gnotepad.svg)
endif()

enable_testing()
add_subdirectory(tests)
