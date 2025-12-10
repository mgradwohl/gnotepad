# UI Test Coverage Requirements

This document defines the comprehensive UI test coverage requirements for GnotePad. All new and modified UI features must include relevant test coverage as outlined below.

## Overview

GnotePad follows a test-driven approach for UI development. Every UI feature must have corresponding automated tests that validate functionality, user interactions, and edge cases. This ensures:

- Robust UX parity with Windows Notepad
- Prevention of regressions
- Reliable cross-platform behavior
- Maintainable codebase

## Test Infrastructure

### Test Framework
- **Qt Test Framework**: Used for all UI tests
- **Headless Mode**: Tests run with `QT_QPA_PLATFORM=offscreen`
- **CMake Integration**: All tests registered via `add_test()` in CMakeLists.txt
- **Test Hooks**: `GNOTE_TEST_HOOKS` define enables test-only APIs in `MainWindow`

### Test Location
- **Directory**: `/tests/smoke/`
- **Test Class**: `MainWindowSmokeTests`
- **Implementation**: `tests/smoke/smoke.cpp`
- **Header**: `tests/smoke/MainWindowSmokeTests.h`

### Running Tests
```bash
# Build tests
cmake --build build/debug

# Run all tests
ctest --test-dir build/debug

# Run specific UI test
build/debug/GnotePadSmoke testMenuActionsExist

# Run tests matching pattern
ctest --test-dir build/debug -R "Dialog|Menu"
```

## Required Test Coverage by Feature

### 1. Interactive Dialogs

#### Find Dialog
- [ ] **testFindDialogInvocation** - Dialog opens correctly
- [x] **testFindNavigation** - Find Next/Previous functionality
- [ ] Test case sensitivity toggle
- [ ] Test wrap-around search behavior
- [ ] Test "not found" message display
- [ ] Test dialog dismissal and state preservation

#### Replace Dialog
- [ ] **testReplaceDialogInvocation** - Dialog opens correctly
- [x] **testReplaceOperations** - Replace and Replace All functionality
- [ ] Test Find Next button in replace dialog
- [ ] Test undo/redo after replace operations
- [ ] Test replacement count message

#### Go To Line Dialog
- [x] **testGoToLineDialog** - Dialog accessibility and validation
- [ ] Test boundary conditions (line 1, last line)
- [ ] Test invalid line number handling
- [ ] Test cursor positioning after navigation
- [ ] Test centering behavior

#### Font Selection Dialog
- [x] **testFontDialogInvocation** - Dialog opens and accepts selection
- [ ] Test font family changes
- [ ] Test font size adjustments
- [ ] Test monospace font filtering
- [ ] Test font persistence across sessions

#### Encoding Selection Dialog
- [x] **testEncodingDialogFlow** - Dialog interaction
- [x] **testSaveAsWithEncoding** - Encoding selection during save
- [ ] Test BOM toggle for UTF-8
- [ ] Test encoding preview
- [ ] Test encoding change warnings

#### Tab Size Dialog
- [x] **testTabSizeDialog** - Dialog invocation
- [ ] Test valid range enforcement (1-16 spaces)
- [ ] Test immediate editor update
- [ ] Test persistence

#### About Dialog
- [x] **testAboutDialog** - Dialog display
- [ ] Test version information display
- [ ] Test hyperlinks functionality
- [ ] Test icon rendering

### 2. Menus and Menu Actions

#### File Menu
- [x] **testMenuActionsExist** - All menu actions present
- [x] **testShortcutCommands** - Keyboard shortcuts work
- [x] **testRecentFilesMenu** - Recent files tracking
- [x] **testRecentFilesMenuActions** - Recent file selection
- [ ] Test New (Ctrl+N) with unsaved changes
- [ ] Test Open (Ctrl+O) file selection
- [ ] Test Save (Ctrl+S) behavior
- [ ] Test Save As dialog
- [ ] Test Print (Ctrl+P) dialog
- [ ] Test Exit with confirmation

#### Edit Menu
- [x] **testEditMenuActionsEnabled** - Action state management
- [ ] Test Undo/Redo (Ctrl+Z/Ctrl+Y)
- [ ] Test Cut/Copy/Paste (Ctrl+X/C/V)
- [ ] Test Delete with selection
- [ ] Test Select All (Ctrl+A)
- [x] **testInsertTimeDate** - Time/Date insertion (F5)

#### Format Menu
- [x] **testWordWrapToggle** - Word wrap toggle
- [ ] Test font selection menu action
- [x] **testTabSizeDialog** - Tab size configuration

#### View Menu
- [x] **testStatusBarToggle** - Status bar visibility
- [x] **testToggleLineNumbers** - Line numbers toggle
- [x] **testZoomActions** - Zoom in/out/reset
- [x] **testDateFormatPreference** - Date format selection

#### Help Menu
- [x] **testAboutDialog** - About dialog
- [ ] Test help documentation link

### 3. Status Bar

- [x] **testStatusBarToggle** - Show/hide status bar
- [x] **testStatusBarLabelsUpdate** - Status labels update correctly
- [ ] Test cursor position display (Ln X, Col Y)
- [x] **testEncodingDialogFlow** - Encoding indicator
- [x] **testZoomLabelUpdates** - Zoom percentage display
- [ ] Test document statistics (lines, words, characters)
- [ ] Test status bar tooltips

### 4. Zoom Controls

- [x] **testZoomActions** - Zoom In/Out/Reset
- [x] **testZoomLabelUpdates** - Zoom label reflects changes
- [ ] Test zoom limits (10% - 500%)
- [ ] Test zoom persistence
- [ ] Test zoom with Ctrl+Wheel
- [ ] Test zoom keyboard shortcuts

### 5. Line Numbers

- [x] **testToggleLineNumbers** - Toggle on/off
- [ ] Test line number gutter rendering
- [ ] Test line number width adjustment
- [ ] Test line number font consistency
- [ ] Test line number persistence

### 6. Action State Management

- [x] **testActionStateManagement** - Actions enable/disable correctly
- [ ] Test Save action state (enabled when modified)
- [ ] Test Save As always enabled
- [ ] Test Print enabled with content
- [ ] Test Cut/Copy enabled with selection
- [ ] Test Paste enabled with clipboard content
- [ ] Test Find actions state

### 7. Keyboard Shortcuts

- [x] **testMenuShortcuts** - Standard shortcuts registered
- [x] **testShortcutCommands** - Shortcuts execute correctly
- [ ] Test Ctrl+N (New)
- [ ] Test Ctrl+O (Open)
- [ ] Test Ctrl+S (Save)
- [ ] Test Ctrl+P (Print)
- [ ] Test Ctrl+F (Find)
- [ ] Test Ctrl+H (Replace)
- [ ] Test Ctrl+G (Go To)
- [ ] Test F5 (Insert Time/Date)
- [ ] Test F3 (Find Next)
- [ ] Test Shift+F3 (Find Previous)
- [ ] Test Ctrl+Plus/Minus (Zoom)

### 8. Tooltips and Help

- [x] **testTooltipPresence** - Tooltips exist where expected
- [ ] Test status bar label tooltips
- [ ] Test toolbar button tooltips (if applicable)
- [ ] Test action tooltips in menus
- [ ] Test context-sensitive help

### 9. Error Dialogs and Messages

- [x] **testFindNavigation** - "Not found" message
- [x] **testReplaceOperations** - Replacement count message
- [x] **testDestructivePrompts** - Save/Discard/Cancel prompts
- [ ] Test file load error handling
- [ ] Test file save error handling
- [ ] Test encoding conversion errors
- [ ] Test permission denied errors
- [ ] Test disk full errors

### 10. Window State and Geometry

- [x] **testWindowStateTransitions** - Minimize/Maximize/Restore
- [x] **testDefaultsWithoutSettings** - Default window size
- [x] **testHandlesCorruptSettings** - Graceful degradation
- [ ] Test window position persistence
- [ ] Test window size persistence
- [ ] Test multi-monitor support
- [ ] Test window state restoration

### 11. Accessibility

- [ ] Test keyboard navigation through menus
- [ ] Test tab order in dialogs
- [ ] Test screen reader labels
- [ ] Test high contrast mode compatibility
- [ ] Test focus indicators
- [ ] Test accessible names for controls
- [ ] Test ARIA roles (if applicable)

### 12. Locale and Internationalization

- [x] **testDateFormatPreference** - Locale-aware date formatting
- [x] **testMultilingualContent** - Multi-language text handling
- [ ] Test UI translation framework
- [ ] Test RTL (Right-to-Left) text support
- [ ] Test locale-specific number formats
- [ ] Test locale-specific date/time formats

### 13. Multi-Document Handling (Future)

When multi-document support is added, the following tests are required:

- [ ] Test tab creation and switching
- [ ] Test tab close with unsaved changes
- [ ] Test tab reordering
- [ ] Test tab persistence
- [ ] Test document switching keyboard shortcuts
- [ ] Test window management (if multi-window)
- [ ] Test document title updates in tabs
- [ ] Test modified indicator in tabs

### 14. PDF Export and Print Preview (Future)

See `PDF_TESTING.md` for detailed PDF testing strategy. Required tests:

- [ ] Test print preview rendering
- [ ] Test page setup configuration
- [ ] Test print dialog invocation
- [ ] Test PDF export with different encodings
- [ ] Test PDF content validation
- [ ] Test print preview zoom
- [ ] Test page layout options

### 15. Visual Regression Testing (Future)

- [ ] Automated screenshot comparison
- [ ] Dialog layout consistency
- [ ] Font rendering validation
- [ ] Theme consistency
- [ ] Icon rendering tests

## Test Patterns and Best Practices

### Test Structure

Each test should follow this structure:

```cpp
void MainWindowSmokeTests::testFeatureName()
{
    // 1. Setup
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    
    // 2. Get necessary components
    auto* editor = window.editorForTest();
    QVERIFY(editor);
    
    // 3. Perform action
    QMetaObject::invokeMethod(&window, "handleSomeAction");
    
    // 4. Verify result
    QTRY_VERIFY(expectedCondition);
    QCOMPARE(actualValue, expectedValue);
}
```

### Using Test Hooks

For testable UI code, use `GNOTE_TEST_HOOKS`:

```cpp
#if defined(GNOTE_TEST_HOOKS)
    // Test-only API
    QAction* findActionForTest() const { return m_findAction; }
    void setAutoDismissDialogsForTest(bool enabled) { m_testAutoDismissDialogs = enabled; }
#endif
```

### Testing Dialogs

For modal dialogs, use auto-dismiss pattern:

```cpp
void MainWindowSmokeTests::testModalDialog()
{
    MainWindow window;
    window.setAutoDismissDialogsForTest(true);
    
    const int countBefore = window.dialogInvocationCountForTest();
    QMetaObject::invokeMethod(&window, "handleShowDialog");
    QTRY_COMPARE(window.dialogInvocationCountForTest(), countBefore + 1);
}
```

### Async Operations

Use `QTRY_*` macros for async checks:

```cpp
// Wait for condition to become true
QTRY_VERIFY(window.isVisible());

// Wait for value to match
QTRY_COMPARE(editor->zoomPercentage(), 100);
```

### File-Based Tests

Use test files from `testfiles/` directory:

```cpp
const QString path = resolveTestFile(QStringLiteral("sample.txt"));
QVERIFY2(!path.isEmpty(), "sample.txt not found");
QVERIFY(window.testLoadDocument(path));
```

## Code Review Checklist

When reviewing UI changes, verify:

- [ ] All new UI features have corresponding tests
- [ ] Tests cover happy path and edge cases
- [ ] Tests include keyboard shortcut validation
- [ ] Tests verify action state management
- [ ] Tests check error message display
- [ ] Tests validate data persistence
- [ ] Test names clearly describe what is tested
- [ ] Tests use appropriate `QVERIFY` and `QCOMPARE` assertions
- [ ] Tests clean up temporary files
- [ ] Tests are platform-independent
- [ ] CMakeLists.txt updated with new test names

## Continuous Integration

All tests must pass in CI before merging:

```bash
# CI test command
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

## Test Coverage Metrics

Current UI test coverage (as of last update):

- **Total UI Tests**: 48 tests
- **Dialog Tests**: 8 tests
- **Menu Tests**: 7 tests
- **Status Bar Tests**: 3 tests
- **Zoom Tests**: 3 tests
- **Encoding Tests**: 9 tests
- **Action State Tests**: 5 tests
- **Window State Tests**: 4 tests
- **Integration Tests**: 9 tests

Target: 90%+ coverage of all user-visible UI features

## Future Enhancements

Planned test infrastructure improvements:

1. **Visual Regression Testing**: Automated screenshot comparison
2. **Performance Testing**: UI responsiveness benchmarks
3. **Stress Testing**: Large file handling, rapid actions
4. **Accessibility Testing**: Automated WCAG compliance checks
5. **Fuzzing**: Random input generation for robustness
6. **Cross-Platform Validation**: Platform-specific test suites

## References

- Qt Test Framework: https://doc.qt.io/qt-6/qtest-overview.html
- Qt Test Macros: https://doc.qt.io/qt-6/qtest.html
- WCAG Accessibility Guidelines: https://www.w3.org/WAI/WCAG21/quickref/
- Windows Notepad UX Reference: https://support.microsoft.com/en-us/windows/help-in-notepad-4d68c388-2ff2-0e7f-b706-35fb2ab88a8c

## Conclusion

Comprehensive UI test coverage is essential for maintaining GnotePad's quality and reliability. All contributors must ensure new features include appropriate tests, and code reviews must explicitly verify test coverage. This document serves as the authoritative reference for UI testing requirements.

For questions or clarifications, refer to existing tests in `tests/smoke/` or consult the CONTRIBUTING.md guide.
