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
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.FileIO.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Settings.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Search.cpp
	${CMAKE_SOURCE_DIR}/src/ui/PrintSupport.cpp
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
	testLoadPrinterSettingsValid
	testLoadPrinterSettingsInvalid
	testSavePrinterSettingsEmpty
	testSavePrinterSettingsNonEmpty
	testPrinterRoundTrip
	testDefaultPrinterBehavior
	testUtf8BomDetection
	testUtf16LEBomDetection
	testUtf16BEBomDetection
	testMultilingualContent
	testUnicodeCharacters
	testEncodingRoundTripUtf8ToBom
	testEncodingRoundTripUtf16Variants
	testLargeFileEncodingRoundTrip
	testMixedContentPreservation
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

# Qt style configuration tests
add_executable(GnotePadStyle
	style/style.cpp
	style/StyleConfigTests.h
	style/StyleConfigTests.cpp
)

set_source_files_properties(
	style/style.cpp
	style/StyleConfigTests.h
	style/StyleConfigTests.cpp
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadStyle PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadStyle)
endif()

target_include_directories(GnotePadStyle PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

set_target_properties(GnotePadStyle PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadStyle PRIVATE
	Qt6::Core
	Qt6::Widgets
	Qt6::Test
)

if(WIN32)
	add_custom_command(TARGET GnotePadStyle POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadStyle>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadStyle>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadStyle>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadStyle>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadStyle.exe"
	)
endif()

set(GNOTE_STYLE_TEST_FUNCTIONS
	testStyleFactoryKeysNotEmpty
	testFusionStyleAvailable
	testWindowsStyleAvailable
	testStyleSelectionDoesNotCrash
)

foreach(test_name IN LISTS GNOTE_STYLE_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadStyle> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_style_all COMMAND $<TARGET_FILE:GnotePadStyle>)

# Performance tests
add_executable(GnotePadPerformance
	performance/performance.cpp
	performance/PerformanceTests.h
)

set_source_files_properties(
	performance/performance.cpp
	performance/PerformanceTests.h
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadPerformance PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadPerformance)
endif()

target_include_directories(GnotePadPerformance PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

target_sources(GnotePadPerformance PRIVATE
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.FileIO.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Settings.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Search.cpp
	${CMAKE_SOURCE_DIR}/src/ui/PrintSupport.cpp
	${CMAKE_SOURCE_DIR}/src/ui/TextEditor.cpp
	${GNOTE_RESOURCES}
)

set_target_properties(GnotePadPerformance PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadPerformance PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Test
	Qt6::PrintSupport
	Qt6::Svg
	Qt6::SvgWidgets
	spdlog::spdlog_header_only
)

target_compile_definitions(GnotePadPerformance PRIVATE GNOTE_TEST_HOOKS)

if(WIN32)
	# Stage Qt runtime and plugins beside the test binary so offscreen platform plugin is found.
	get_filename_component(QT6_PREFIX "${Qt6_DIR}/../.." ABSOLUTE)
	set(QT6_DEBUG_BIN "${QT6_PREFIX}/debug/bin")
	set(QT6_RELEASE_BIN "${QT6_PREFIX}/bin")
	set(QT6_DEBUG_PLUGINS "${QT6_PREFIX}/debug/Qt6/plugins")
	set(QT6_RELEASE_PLUGINS "${QT6_PREFIX}/Qt6/plugins")

	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf" CONTENT "[Paths]\nPlugins=plugins\n")
	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf" CONTENT "[Paths]\nPlugins=plugins\n")

	add_custom_command(TARGET GnotePadPerformance POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadPerformance>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadPerformance>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadPerformance>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadPerformance>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadPerformance.exe"
	)
endif()

add_custom_command(TARGET GnotePadPerformance POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		$<TARGET_FILE_DIR:GnotePadPerformance>/testfiles/performance
)

set(GNOTE_PERFORMANCE_TEST_FUNCTIONS
	testLoadLargeFile100KB
	testLoadLargeFile500KB
	testLoadLargeFile1MB
	testScrollPerformanceLargeFile
	testEditPerformanceLargeFile
	testSaveLargeFile
	testEncodingConversionUtf8ToUtf16LE
	testEncodingConversionUtf8ToUtf16BE
	testEncodingRoundTripPerformance
	testMemoryUsageDuringEncoding
	testFindPerformanceLargeFile
	testReplacePerformanceLargeFile
	testUndoRedoStackPerformance
	testZoomOperationsPerformance
	testDocumentModificationPerformance
	testMassiveInsertOperations
	testMassiveDeleteOperations
	testContinuousEditing
)

foreach(test_name IN LISTS GNOTE_PERFORMANCE_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_perf_${test_name}
		COMMAND $<TARGET_FILE:GnotePadPerformance> ${test_name}
	)
	# Performance tests are labeled separately so they can be run independently
	set_tests_properties(gnotepad_perf_${test_name} PROPERTIES LABELS "Performance")
endforeach()

add_test(NAME gnotepad_performance_all COMMAND $<TARGET_FILE:GnotePadPerformance>)
set_tests_properties(gnotepad_performance_all PROPERTIES LABELS "Performance")
