add_executable(GnotePadSmoke
	smoke/smoke.cpp
	smoke/MainWindowSmokeTests.h
)

set_source_files_properties(
	smoke/smoke.cpp
	smoke/MainWindowSmokeTests.h
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadSmoke PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadSmoke)
endif()

target_include_directories(GnotePadSmoke PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

target_sources(GnotePadSmoke PRIVATE
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
	${CMAKE_SOURCE_DIR}/src/ui/TextEditor.cpp
	${GNOTE_RESOURCES}
)

set_target_properties(GnotePadSmoke PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadSmoke PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Test
	Qt6::PrintSupport
	Qt6::Svg
	Qt6::SvgWidgets
	spdlog::spdlog_header_only
)

target_compile_definitions(GnotePadSmoke PRIVATE GNOTE_TEST_HOOKS)

if(WIN32)
	# Stage Qt runtime and plugins beside the test binary so offscreen platform plugin is found.
	get_filename_component(QT6_PREFIX "${Qt6_DIR}/../.." ABSOLUTE)
	set(QT6_DEBUG_BIN "${QT6_PREFIX}/debug/bin")
	set(QT6_RELEASE_BIN "${QT6_PREFIX}/bin")
	set(QT6_DEBUG_PLUGINS "${QT6_PREFIX}/debug/Qt6/plugins")
	set(QT6_RELEASE_PLUGINS "${QT6_PREFIX}/Qt6/plugins")

	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf" CONTENT "[Paths]\nPlugins=plugins\n")
	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf" CONTENT "[Paths]\nPlugins=plugins\n")

	add_custom_command(TARGET GnotePadSmoke POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadSmoke>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadSmoke>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadSmoke>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadSmoke>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadSmoke.exe"
	)
endif()

add_custom_command(TARGET GnotePadSmoke POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/tests/testfiles
		$<TARGET_FILE_DIR:GnotePadSmoke>/testfiles
)

set(GNOTE_SMOKE_TEST_FUNCTIONS
	testLaunchShowsWindow
	testWindowStateTransitions
	testDefaultsWithoutSettings
	testHandlesCorruptSettings
	testZoomActions
	testInsertTimeDate
	testToggleLineNumbers
	testOpenSampleFile
	testLargeFileScrolling
	testSaveAsWithEncoding
	testEncodingRoundTripVariants
	testFindNavigation
	testReplaceOperations
	testRecentFilesMenu
	testDestructivePrompts
	testShortcutCommands
)

foreach(test_name IN LISTS GNOTE_SMOKE_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadSmoke> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_smoke_all COMMAND $<TARGET_FILE:GnotePadSmoke>)

# Command-line parsing tests
add_executable(GnotePadCmdLine
	cmdline/cmdline.cpp
	cmdline/ApplicationCmdLineTests.h
)

set_source_files_properties(
	cmdline/cmdline.cpp
	cmdline/ApplicationCmdLineTests.h
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadCmdLine PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadCmdLine)
endif()

target_include_directories(GnotePadCmdLine PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

set_target_properties(GnotePadCmdLine PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadCmdLine PRIVATE
	Qt6::Core
	Qt6::Test
)

if(WIN32)
	add_custom_command(TARGET GnotePadCmdLine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadCmdLine>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadCmdLine>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadCmdLine>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadCmdLine>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadCmdLine.exe"
	)
endif()

set(GNOTE_CMDLINE_TEST_FUNCTIONS
	testQuitAfterInitParsing
	testHeadlessSmokeParsing
	testNoFlagsParsing
	testQuitAfterInitBehavior
)

foreach(test_name IN LISTS GNOTE_CMDLINE_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadCmdLine> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_cmdline_all COMMAND $<TARGET_FILE:GnotePadCmdLine>)
