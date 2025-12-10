# Testing Guidelines for GnotePad

This document outlines the testing requirements and best practices for GnotePad development.

## Test Philosophy

All functional changes to GnotePad should include relevant tests. Tests ensure:
- Features work as intended
- Regressions are caught early
- Code remains maintainable
- Edge cases are handled gracefully

## Test Categories

### 1. Command-Line Tests (`tests/cmdline/`)

Tests for command-line argument parsing and handling.

**Required tests for new command-line options:**
- Flag parsing (short and long forms)
- Invalid flag handling
- Flag combinations
- Expected behavior verification

**Example:**
```cpp
void testNewOption()
{
    QCommandLineParser parser;
    setupParser(parser);
    
    QStringList args;
    args << QStringLiteral("GnotePad") << QStringLiteral("--new-flag");
    
    QVERIFY(parser.parse(args));
    QVERIFY(parser.isSet(QStringLiteral("new-flag")));
}
```

### 2. Smoke Tests (`tests/smoke/`)

High-level integration tests for core functionality.

**Coverage includes:**
- Window lifecycle (show, hide, minimize, maximize)
- Document loading and saving
- Encoding detection and conversion
- Find/replace operations
- Recent files management
- Settings persistence
- Printer configuration

**When to add smoke tests:**
- New file format support
- New encoding support
- New UI workflows
- Major feature additions

### 3. Menu Action Tests (`tests/menuactions/`)

Tests for menu actions and their enable/disable states.

**Required tests for new menu actions:**
- Action enabled states based on document state
- Action triggered behavior
- Error handling (graceful failures)
- Keyboard shortcuts (if applicable)

**Example:**
```cpp
void testNewMenuAction()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    
    // Test action exists
    QAction* action = findActionByName(&window, "ActionName");
    QVERIFY(action);
    
    // Test enabled state
    QVERIFY(action->isEnabled());
    
    // Trigger action and verify behavior
    action->trigger();
    QApplication::processEvents();
    
    // Verify expected outcome
    QVERIFY(/* expected condition */);
}
```

### 4. Encoding Tests (`tests/encoding/`)

Tests for encoding detection, conversion, and edge cases.

**Coverage includes:**
- Empty files
- BOM detection (UTF-8, UTF-16LE, UTF-16BE)
- Mixed line endings
- Incomplete UTF-8 sequences
- Encoding conversion round-trips
- Large files
- Null bytes and special characters

**When to add encoding tests:**
- New encoding support
- Changes to BOM detection logic
- Changes to file loading/saving

### 5. Style Tests (`tests/style/`)

Tests for Qt style configuration and platform-specific behavior.

## Writing Good Tests

### Test Structure

Each test should follow the Arrange-Act-Assert pattern:

```cpp
void testSomething()
{
    // Arrange: Set up test conditions
    MainWindow window;
    auto* editor = window.editorForTest();
    
    // Act: Perform the operation
    editor->setPlainText(QStringLiteral("test"));
    
    // Assert: Verify the outcome
    QCOMPARE(editor->toPlainText(), QStringLiteral("test"));
}
```

### Best Practices

1. **Independence**: Tests should not depend on each other
2. **Cleanup**: Use QTemporaryDir for file operations
3. **Async handling**: Use QTRY_VERIFY/QTRY_COMPARE for UI operations
4. **Test mode**: Enable test mode with `QStandardPaths::setTestModeEnabled(true)`
5. **Descriptive names**: Test names should clearly indicate what is being tested
6. **Edge cases**: Always test boundary conditions and error cases

### Common Patterns

**File operations:**
```cpp
QTemporaryDir tempDir;
QVERIFY(tempDir.isValid());
const QString path = tempDir.filePath(QStringLiteral("test.txt"));
```

**UI operations:**
```cpp
MainWindow window;
window.show();
QTRY_VERIFY(window.isVisible()); // Wait for window to appear
QApplication::processEvents();    // Process pending events
```

**Error handling:**
```cpp
// Test that invalid input is handled gracefully
bool result = window.testLoadDocument("/nonexistent/file.txt");
QVERIFY(!result); // Should fail
// Verify application is still functional
QVERIFY(window.editorForTest());
```

## Test Hooks

The MainWindow class provides test hooks when compiled with `GNOTE_TEST_HOOKS`:

```cpp
#if defined(GNOTE_TEST_HOOKS)
    bool testLoadDocument(const QString& path);
    bool testSaveDocument(const QString& path);
    TextEditor* editorForTest() const;
    QAction* findActionForTest() const;
    // ... and more
#endif
```

**Using test hooks:**
```cpp
MainWindow window;
QVERIFY(window.testLoadDocument("test.txt"));
auto* editor = window.editorForTest();
QVERIFY(!editor->toPlainText().isEmpty());
```

## Running Tests

### Run all tests:
```bash
ctest --test-dir build/debug
```

### Run specific test suite:
```bash
ctest --test-dir build/debug -R cmdline
ctest --test-dir build/debug -R smoke
ctest --test-dir build/debug -R menuactions
ctest --test-dir build/debug -R encoding
```

### Run specific test:
```bash
./build/debug/GnotePadCmdLine testHelpOption
./build/debug/GnotePadSmoke testEncodingRoundTripVariants
```

### Run with verbose output:
```bash
ctest --test-dir build/debug --verbose
ctest --test-dir build/debug --output-on-failure
```

## Code Review Checklist

When reviewing changes, ensure:

- [ ] New features include relevant tests
- [ ] Tests cover normal and error cases
- [ ] Tests are independent and repeatable
- [ ] Test names are descriptive
- [ ] Tests use appropriate assertions (QVERIFY, QCOMPARE, etc.)
- [ ] Async operations use QTRY_* macros
- [ ] File operations use QTemporaryDir
- [ ] Tests pass locally before submission
- [ ] Tests are added to CMakeLists.txt

## Missing Test Categories to Add

If you're adding these features, please include tests:

- **Multi-document handling**: Window management, document switching
- **Page setup**: Margins, orientation, paper size
- **Print preview**: Preview rendering, page navigation
- **PDF export**: Export functionality, encoding preservation
- **Clipboard operations**: Copy, cut, paste with different formats
- **Undo/Redo**: Complex undo scenarios, redo after modifications
- **Font selection**: Font rendering, fallback fonts

## Resources

- Qt Test documentation: https://doc.qt.io/qt-6/qtest.html
- Qt Test tutorial: https://doc.qt.io/qt-6/qtest-tutorial.html
- Google Test best practices: https://google.github.io/googletest/
- Testing best practices: https://martinfowler.com/articles/practical-test-pyramid.html

## Contributing

When proposing new features:
1. Include tests in the initial implementation
2. If tests are not feasible, document why in the PR
3. Reference this guide when reviewing code
4. Suggest missing tests when reviewing PRs

Remember: **Code without tests is legacy code.**
