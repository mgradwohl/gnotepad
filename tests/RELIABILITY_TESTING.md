# Reliability Testing Guide for GnotePad

This document provides guidance for implementing and maintaining reliability tests in GnotePad, ensuring the application behaves correctly under various conditions including edge cases, error scenarios, and state persistence.

## Overview

The reliability test suite (`tests/reliability/`) validates GnotePad's behavior in areas critical to user trust and data integrity:

- **MRU (Most Recently Used) file handling** - Ensuring file history is maintained correctly
- **Settings persistence** - Validating preferences survive application restarts
- **Error scenarios** - Verifying graceful handling of file system errors
- **Window state management** - Testing stability across minimize/maximize/restore cycles

## Running Reliability Tests

```bash
# Build reliability tests
cmake --build build/debug --target GnotePadReliability

# Run all reliability tests
ctest --test-dir build/debug -R "gnotepad_test"

# Run specific reliability test
./build/debug/tests/GnotePadReliability testMruPersistenceAcrossRestarts

# Run all tests
./build/debug/tests/GnotePadReliability
```

## Test Categories

### 1. MRU File Handling (6 tests)

These tests validate the Most Recently Used file list functionality:

- **testMruPersistenceAcrossRestarts**: MRU list is loaded from QSettings on startup
- **testMruUpdateOnFileOpen**: Opening files updates MRU list in correct order
- **testMruCleanupNonExistentFiles**: Non-existent files are handled gracefully
- **testMruMaximumSizeEnforcement**: List is capped at 10 entries
- **testMruDuplicateEntries**: Duplicate files are prevented
- **testMruFileOrdering**: Files are ordered chronologically (most recent first)

### 2. Settings Persistence (6 tests)

These tests validate QSettings persistence across application restarts:

- **testSettingsPersistenceAcrossRestarts**: User preferences are restored
- **testSettingsRecoveryFromMissingFile**: Defaults are used when settings missing
- **testSettingsRecoveryFromCorruptData**: Corrupt settings are handled gracefully
- **testWindowGeometryPersistence**: Window size and position are restored
- **testEditorStatePersistence**: Editor state (zoom, tabs, etc.) is restored
- **testEncodingPreferencePersistence**: Encoding settings are maintained

### 3. Error Scenarios (7 tests)

These tests validate error handling for file system operations:

- **testOpenFilePermissionDenied**: Graceful handling of permission errors (SKIPPED - modal dialog)
- **testSaveFilePermissionDenied**: Proper error when saving without permissions (SKIPPED - modal dialog)
- **testSaveToReadOnlyFile**: Read-only file save attempts handled (SKIPPED - modal dialog)
- **testSaveToInvalidPath**: Invalid path save attempts handled (SKIPPED - modal dialog)
- **testLoadNonExistentFile**: Non-existent file loads handled (SKIPPED - modal dialog)
- **testLoadBinaryFile**: Binary file loads don't crash application (SKIPPED - modal dialog)
- **testSaveWithInsufficientSpace**: Disk full scenarios (SKIPPED - platform-specific)

**Note**: Several error scenario tests are skipped in automated runs because they trigger modal dialogs that block execution in headless environments. These scenarios should be tested manually or with a future enhanced test infrastructure that can intercept dialogs.

### 4. Window State and Stability (7 tests)

These tests validate window state management:

- **testWindowPositionPersistence**: Window position restored from settings
- **testWindowStateStability**: Normal/minimized/maximized state transitions work
- **testRapidWindowStateChanges**: Rapid state changes don't cause crashes
- **testCloseWithUnsavedChanges**: Unsaved changes prompt works correctly
- **testMultipleWindowStateCycles**: Multiple open/close cycles are stable
- **testMinimizeRestoreContent**: Content preserved through minimize/restore
- **testMaximizeRestoreContent**: Content preserved through maximize/restore

## Best Practices for Reliability Testing

### 1. Settings Persistence Testing

When testing settings persistence, simulate application restarts by:

```cpp
// Write settings manually to simulate previous session
{
    QSettings settings;
    settings.setValue("editor/zoomPercent", 130);
    settings.setValue("editor/lineNumbersVisible", false);
    settings.sync();
}

// Create new window - should load settings
{
    MainWindow window;
    // Verify settings were loaded
    QCOMPARE(editor->zoomPercentage(), 130);
}
```

**Do NOT** try to save settings by destroying windows - settings are only saved on closeEvent, which isn't reliably triggered in automated tests.

### 2. Avoiding Modal Dialogs in Tests

File I/O errors (permission denied, file not found) trigger `QMessageBox::warning()` which blocks test execution. For these scenarios:

```cpp
// SKIP the test with documentation
QSKIP("Test would show blocking modal dialog in headless environment");
```

Future enhancements could:
- Add a test mode that converts modal dialogs to non-blocking
- Implement signal-based error notification
- Mock QMessageBox in test builds

### 3. Window Geometry Testing

Window managers may add decorations or enforce constraints. Use tolerant comparisons:

```cpp
// Allow small deviation for window manager effects
QVERIFY(qAbs(geometry.x() - expected.x()) <= 5);
QVERIFY(qAbs(geometry.y() - expected.y()) <= 5);
```

### 4. Temporary File Management

Always clean up temporary files and use proper patterns:

```cpp
// Create temp file that won't auto-delete
QTemporaryFile temp(m_tempDir + "/testfile_XXXXXX.txt");
temp.setAutoRemove(false);
temp.open();
temp.write("content");
QString fileName = temp.fileName();
temp.close();

// Use file in tests...

// Clean up explicitly
QFile::remove(fileName);
```

### 5. Settings Isolation

Always clear settings before tests to ensure clean state:

```cpp
void clearAllSettings()
{
    QSettings settings;
    settings.clear();
    settings.sync();
    
    const QString filePath = settings.fileName();
    QFile::remove(filePath);
    QFile::remove(filePath + QStringLiteral(".lock"));
}
```

## Adding New Reliability Tests

When adding functionality that affects reliability:

1. **Identify reliability concerns**: What could go wrong? What needs to persist?

2. **Create focused tests**: Each test should verify one specific reliability aspect

3. **Consider error scenarios**: How should the feature behave when:
   - Settings are missing or corrupt?
   - File operations fail?
   - System resources are limited?
   - State transitions occur rapidly?

4. **Document skip reasons**: If a test must be skipped, document why clearly

5. **Update CMakeLists.txt**: Add new test functions to `GNOTE_RELIABILITY_TEST_FUNCTIONS`

### Example Test Structure

```cpp
void ReliabilityTests::testNewFeaturePersistence()
{
    clearAllSettings();
    
    const int expectedValue = 42;
    
    // Simulate previous session
    {
        QSettings settings;
        settings.setValue("feature/value", expectedValue);
        settings.sync();
    }
    
    // Verify new session loads value
    {
        MainWindow window;
        // Verify value was loaded and applied
        QCOMPARE(window.getFeatureValue(), expectedValue);
    }
}
```

## Common Pitfalls

### ❌ Don't: Try to save settings by destroying windows

```cpp
// BAD: Settings won't be saved
{
    MainWindow window;
    editor->setZoomPercentage(150);
} // Settings NOT saved - closeEvent not called
```

### ✅ Do: Write settings manually for testing

```cpp
// GOOD: Explicitly write settings
{
    QSettings settings;
    settings.setValue("editor/zoomPercent", 150);
    settings.sync();
}
```

### ❌ Don't: Expect exact window positions

```cpp
// BAD: May fail due to window manager
QCOMPARE(window.pos(), expectedPos);
```

### ✅ Do: Use tolerant comparisons

```cpp
// GOOD: Allows for window manager effects
QVERIFY(qAbs(window.x() - expectedPos.x()) <= 5);
```

### ❌ Don't: Test file errors that show dialogs

```cpp
// BAD: Will hang in automated tests
window.testLoadDocument("/nonexistent/file.txt");
```

### ✅ Do: Skip tests that would block

```cpp
// GOOD: Document why skipped
QSKIP("Test would show blocking modal dialog");
```

## Integration with CI/CD

Reliability tests are integrated into the standard CTest suite:

```bash
# All tests including reliability
ctest --test-dir build/debug

# Only reliability tests
ctest --test-dir build/debug -R "testMru|testSettings|testWindow"
```

Expected results:
- ~20 tests PASS
- ~7 tests SKIPPED (modal dialog blockers)
- 0 tests FAIL

## Future Enhancements

Potential improvements to reliability testing:

1. **Dialog Interception**: Mock or intercept QMessageBox to enable error scenario testing
2. **Settings Migration Tests**: Test upgrade paths from old settings formats
3. **Platform-Specific Tests**: Conditional tests for Windows/Linux/macOS-specific features
4. **Crash Recovery**: Test document recovery after abnormal termination
5. **Performance Regression**: Add benchmarks for large file handling
6. **Multi-Instance**: Test behavior with multiple GnotePad instances

## References

- **Qt Test Framework**: https://doc.qt.io/qt-6/qtest-overview.html
- **QSettings Documentation**: https://doc.qt.io/qt-6/qsettings.html
- **Qt Test Best Practices**: https://doc.qt.io/qt-6/qttest-index.html

## Questions?

For questions about reliability testing:
1. Review existing tests in `tests/reliability/reliability.cpp`
2. Check this document's Best Practices section
3. Consult Qt Test documentation
4. Ask in project discussions or pull request reviews

---

**Last Updated**: 2025-12-10

**Maintainer**: GnotePad Development Team
