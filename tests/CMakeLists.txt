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
		COMMAND GnotePadSmoke ${test_name}
	)
endforeach()

add_test(NAME gnotepad_smoke_all COMMAND GnotePadSmoke)
