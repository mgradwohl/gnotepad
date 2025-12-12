cmake_minimum_required(VERSION 3.26)

project(GnotePad VERSION 0.8.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep clangd fed regardless of config: copy compile_commands.json to source root on build.
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    add_custom_target(copy-compile-commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
        BYPRODUCTS "${CMAKE_SOURCE_DIR}/compile_commands.json"
        COMMENT "Copying compile_commands.json to source tree for clangd"
    )
endif()

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

find_program(CLANG_TIDY_EXE NAMES clang-tidy
    HINTS "$ENV{LLVM_ROOT}/bin" "$ENV{ProgramFiles}/LLVM/bin" "C:/Program Files/LLVM/bin"
)
find_program(CLANG_FORMAT_EXE NAMES clang-format
    HINTS "$ENV{LLVM_ROOT}/bin" "$ENV{ProgramFiles}/LLVM/bin" "C:/Program Files/LLVM/bin"
)

if(GNOTE_ENABLE_CLANG_TIDY)
    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR "GNOTE_ENABLE_CLANG_TIDY is ON but clang-tidy was not found.")
    endif()
endif()

if(GNOTE_ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
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

# Prefer header-only spdlog and std::format to sidestep MSVC fmt deprecation noise.
set(SPDLOG_HEADER_ONLY ON CACHE BOOL "Build spdlog in header-only mode" FORCE)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)

FetchContent_MakeAvailable(spdlog)

if(TARGET spdlog_header_only)
    target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_USE_STD_FORMAT $<$<PLATFORM_ID:Windows>:_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING>)
endif()

if(TARGET spdlog)
    target_compile_definitions(spdlog PUBLIC SPDLOG_USE_STD_FORMAT $<$<PLATFORM_ID:Windows>:_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING>)
endif()

set(GNOTE_SOURCES
    src/gnotepad.cpp
    src/app/Application.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.FileIO.cpp
    src/ui/MainWindow.Settings.cpp
    src/ui/MainWindow.Search.cpp
    src/ui/PrintSupport.cpp
    src/ui/TextEditor.cpp
)

set(GNOTE_HEADERS
    src/app/Application.h
    src/ui/MainWindow.h
    src/ui/PrintSupport.h
    src/ui/TextEditor.h
)

qt_add_resources(GNOTE_RESOURCES resources/gnotepad.qrc)

if(GNOTE_RESOURCES)
    foreach(resource_source IN LISTS GNOTE_RESOURCES)
        set_source_files_properties(${resource_source} PROPERTIES SKIP_CLANG_TIDY TRUE)
    endforeach()
endif()

if(CLANG_TIDY_EXE)
    add_custom_target(run-clang-tidy
        COMMAND ${CLANG_TIDY_EXE}
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            -p ${CMAKE_BINARY_DIR}
            --extra-arg=-std=c++23
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

# Add Windows application manifest for DPI awareness and other settings
if(WIN32)
    target_sources(GnotePad PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.manifest")
endif()

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
    set_target_properties(GnotePad PROPERTIES WIN32_EXECUTABLE TRUE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_options(GnotePad PRIVATE "-Xlinker" "/ENTRY:wWinMainCRTStartup")
    else()
        target_link_options(GnotePad PRIVATE "/ENTRY:wWinMainCRTStartup")
    endif()
endif()

set_target_properties(GnotePad PROPERTIES
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

if(WIN32)
    # Derive Qt prefix to locate debug/release runtime paths (vcpkg layout: installed/x64-windows[/debug]).
    get_filename_component(QT6_PREFIX "${Qt6_DIR}/../.." ABSOLUTE)
    set(QT6_DEBUG_BIN "${QT6_PREFIX}/debug/bin")
    set(QT6_RELEASE_BIN "${QT6_PREFIX}/bin")
    set(QT6_DEBUG_PLUGINS "${QT6_PREFIX}/debug/Qt6/plugins")
    set(QT6_RELEASE_PLUGINS "${QT6_PREFIX}/Qt6/plugins")

    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.debug.conf" CONTENT "[Paths]\nPlugins=plugins\n")
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.release.conf" CONTENT "[Paths]\nPlugins=plugins\n")

    # Copy Qt runtime DLLs and plugins next to the executable so it can run outside VS Code without env vars.
    add_custom_command(TARGET GnotePad POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePad>/plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<IF:$<CONFIG:Debug>,${QT6_DEBUG_BIN},${QT6_RELEASE_BIN}>
            "$<TARGET_FILE_DIR:GnotePad>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<IF:$<CONFIG:Debug>,${QT6_DEBUG_PLUGINS},${QT6_RELEASE_PLUGINS}>
            "$<TARGET_FILE_DIR:GnotePad>/plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<IF:$<CONFIG:Debug>,${CMAKE_CURRENT_BINARY_DIR}/qt.debug.conf,${CMAKE_CURRENT_BINARY_DIR}/qt.release.conf>
            "$<TARGET_FILE_DIR:GnotePad>/qt.conf"
        COMMENT "Copying Qt runtime and plugins beside GnotePad.exe"
    )
endif()

install(TARGETS GnotePad
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(UNIX AND NOT APPLE)
    set(GNOTE_DESKTOP_FILE ${CMAKE_CURRENT_BINARY_DIR}/app.gnotepad.GnotePad.desktop)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/app.gnotepad.GnotePad.desktop.in ${GNOTE_DESKTOP_FILE} @ONLY)
    install(FILES ${GNOTE_DESKTOP_FILE} DESTINATION share/applications)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/gnotepad-icon.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME app.gnotepad.GnotePad.svg)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app.gnotepad.GnotePad.png
        DESTINATION share/icons/hicolor/256x256/apps)
endif()

set(CPACK_PACKAGE_NAME "gnotepad")
set(CPACK_PACKAGE_VENDOR "GnotePad")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GnotePad - Qt-based text editor")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/mgradwohl/GnotePad")
set(CPACK_PACKAGE_CONTACT "GnotePad contributors")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_STRIP_FILES TRUE)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "GnotePad contributors")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Text")
set(CPACK_RPM_PACKAGE_URL "https://github.com/mgradwohl/GnotePad")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/applications" "/usr/share/icons")

include(CPack)

enable_testing()
add_subdirectory(tests)
