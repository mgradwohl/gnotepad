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
	testHelpOption
	testVersionOption
	testInvalidOption
)

foreach(test_name IN LISTS GNOTE_CMDLINE_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadCmdLine> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_cmdline_all COMMAND $<TARGET_FILE:GnotePadCmdLine>)

# Menu actions tests
add_executable(GnotePadMenuActions
	menuactions/menuactions.cpp
	menuactions/MenuActionsTests.h
)

set_source_files_properties(
	menuactions/menuactions.cpp
	menuactions/MenuActionsTests.h
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadMenuActions PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadMenuActions)
endif()

target_include_directories(GnotePadMenuActions PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

target_sources(GnotePadMenuActions PRIVATE
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.FileIO.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Settings.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Search.cpp
	${CMAKE_SOURCE_DIR}/src/ui/PrintSupport.cpp
	${CMAKE_SOURCE_DIR}/src/ui/TextEditor.cpp
	${GNOTE_RESOURCES}
)

set_target_properties(GnotePadMenuActions PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadMenuActions PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Test
	Qt6::PrintSupport
	Qt6::Svg
	Qt6::SvgWidgets
	spdlog::spdlog_header_only
)

target_compile_definitions(GnotePadMenuActions PRIVATE GNOTE_TEST_HOOKS)

if(WIN32)
	add_custom_command(TARGET GnotePadMenuActions POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadMenuActions>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadMenuActions>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadMenuActions>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadMenuActions>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadMenuActions.exe"
	)
endif()

set(GNOTE_MENUACTIONS_TEST_FUNCTIONS
	testNewFileAction
	testSaveActionEnabledStates
	testSaveAsActionEnabledStates
	testPrintActionEnabledStates
	testCutCopyDeleteEnabledStates
	testSelectAllAction
	testUndoRedoActions
	testFindActionEnabledStates
	testReplaceActionEnabledStates
	testGoToLineActionEnabledStates
	testFindNextPreviousEnabledStates
	testZoomActionsEnabled
	testStatusBarToggle
	testLineNumberToggle
	testWordWrapToggle
	testTimeDateActionEnabledStates
	testOpenNonExistentFile
	testSaveToReadOnlyLocation
)

foreach(test_name IN LISTS GNOTE_MENUACTIONS_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadMenuActions> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_menuactions_all COMMAND $<TARGET_FILE:GnotePadMenuActions>)

# Encoding edge case tests
add_executable(GnotePadEncoding
	encoding/encoding.cpp
	encoding/EncodingEdgeCasesTests.h
)

set_source_files_properties(
	encoding/encoding.cpp
	encoding/EncodingEdgeCasesTests.h
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadEncoding PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadEncoding)
endif()

target_include_directories(GnotePadEncoding PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

target_sources(GnotePadEncoding PRIVATE
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.FileIO.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Settings.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Search.cpp
	${CMAKE_SOURCE_DIR}/src/ui/PrintSupport.cpp
	${CMAKE_SOURCE_DIR}/src/ui/TextEditor.cpp
	${GNOTE_RESOURCES}
)

set_target_properties(GnotePadEncoding PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadEncoding PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Test
	Qt6::PrintSupport
	Qt6::Svg
	Qt6::SvgWidgets
	spdlog::spdlog_header_only
)

target_compile_definitions(GnotePadEncoding PRIVATE GNOTE_TEST_HOOKS)

if(WIN32)
	add_custom_command(TARGET GnotePadEncoding POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadEncoding>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadEncoding>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadEncoding>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadEncoding>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadEncoding.exe"
	)
endif()

add_custom_command(TARGET GnotePadEncoding POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/tests/testfiles
		$<TARGET_FILE_DIR:GnotePadEncoding>/testfiles
)

set(GNOTE_ENCODING_TEST_FUNCTIONS
	testEmptyFileEncoding
	testSingleByteFile
	testLargeUtf8File
	testMixedLineEndings
	testTrailingNewlines
	testNullBytesHandling
	testIncompleteUtf8Sequences
	testBomWithoutContent
	testMultipleBomMarkers
	testEncodingConversionErrors
	testUnsupportedEncoding
)

foreach(test_name IN LISTS GNOTE_ENCODING_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadEncoding> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_encoding_all COMMAND $<TARGET_FILE:GnotePadEncoding>)

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

# Reliability tests
add_executable(GnotePadReliability
	reliability/reliability.cpp
	reliability/ReliabilityTests.h
)

set_source_files_properties(
	reliability/reliability.cpp
	reliability/ReliabilityTests.h
	PROPERTIES SKIP_CLANG_TIDY TRUE
)

target_compile_features(GnotePadReliability PRIVATE cxx_std_23)

if(COMMAND gnote_apply_default_warnings)
	gnote_apply_default_warnings(GnotePadReliability)
endif()

target_include_directories(GnotePadReliability PRIVATE
	${CMAKE_SOURCE_DIR}/src
)

target_sources(GnotePadReliability PRIVATE
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.FileIO.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Settings.cpp
	${CMAKE_SOURCE_DIR}/src/ui/MainWindow.Search.cpp
	${CMAKE_SOURCE_DIR}/src/ui/PrintSupport.cpp
	${CMAKE_SOURCE_DIR}/src/ui/TextEditor.cpp
	${GNOTE_RESOURCES}
)

set_target_properties(GnotePadReliability PROPERTIES AUTOMOC ON)

target_link_libraries(GnotePadReliability PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Test
	Qt6::PrintSupport
	Qt6::Svg
	Qt6::SvgWidgets
	spdlog::spdlog_header_only
)

target_compile_definitions(GnotePadReliability PRIVATE GNOTE_TEST_HOOKS)

if(WIN32)
	# Stage Qt runtime and plugins beside the test binary so offscreen platform plugin is found.
	get_filename_component(QT6_PREFIX "${Qt6_Dir}/../.." ABSOLUTE)
	set(QT6_DEBUG_BIN "${QT6_PREFIX}/debug/bin")
	set(QT6_RELEASE_BIN "${QT6_PREFIX}/bin")
	set(QT6_DEBUG_PLUGINS "${QT6_PREFIX}/debug/Qt6/plugins")
	set(QT6_RELEASE_PLUGINS "${QT6_PREFIX}/Qt6/plugins")

	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf" CONTENT "[Paths]\nPlugins=plugins\n")
	file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf" CONTENT "[Paths]\nPlugins=plugins\n")

	add_custom_command(TARGET GnotePadReliability POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GnotePadReliability>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_BIN}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_BIN}>
			"$<TARGET_FILE_DIR:GnotePadReliability>"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			$<$<CONFIG:Debug>:${QT6_DEBUG_PLUGINS}>
			$<$<NOT:$<CONFIG:Debug>>:${QT6_RELEASE_PLUGINS}>
			"$<TARGET_FILE_DIR:GnotePadReliability>/plugins"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.debug.conf>
			$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_BINARY_DIR}/qt.tests.release.conf>
			"$<TARGET_FILE_DIR:GnotePadReliability>/qt.conf"
		COMMENT "Copying Qt runtime and plugins beside GnotePadReliability.exe"
	)
endif()

add_custom_command(TARGET GnotePadReliability POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/tests/testfiles
		$<TARGET_FILE_DIR:GnotePadReliability>/testfiles
)

set(GNOTE_RELIABILITY_TEST_FUNCTIONS
	testMruPersistenceAcrossRestarts
	testMruUpdateOnFileOpen
	testMruCleanupNonExistentFiles
	testMruMaximumSizeEnforcement
	testMruDuplicateEntries
	testMruFileOrdering
	testSettingsPersistenceAcrossRestarts
	testSettingsRecoveryFromMissingFile
	testSettingsRecoveryFromCorruptData
	testWindowGeometryPersistence
	testEditorStatePersistence
	testEncodingPreferencePersistence
	testOpenFilePermissionDenied
	testSaveFilePermissionDenied
	testSaveToReadOnlyFile
	testSaveToInvalidPath
	testLoadNonExistentFile
	testLoadBinaryFile
	testSaveWithInsufficientSpace
	testWindowPositionPersistence
	testWindowStateStability
	testRapidWindowStateChanges
	testCloseWithUnsavedChanges
	testMultipleWindowStateCycles
	testMinimizeRestoreContent
	testMaximizeRestoreContent
)

foreach(test_name IN LISTS GNOTE_RELIABILITY_TEST_FUNCTIONS)
	add_test(
		NAME gnotepad_${test_name}
		COMMAND $<TARGET_FILE:GnotePadReliability> ${test_name}
	)
endforeach()

add_test(NAME gnotepad_reliability_all COMMAND $<TARGET_FILE:GnotePadReliability>)
