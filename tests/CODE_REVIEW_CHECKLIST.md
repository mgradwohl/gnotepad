# Code Review Guidelines for UI Test Coverage

This document provides guidelines for reviewers to ensure all UI changes include appropriate test coverage as required by the UI Test Improvements initiative.

## Overview

Every pull request that modifies or adds UI features **must** include relevant test coverage. This document helps reviewers verify that test coverage is complete and follows best practices.

## Quick Checklist for Reviewers

When reviewing a PR with UI changes, verify:

- [ ] **Tests exist** for new/modified UI features
- [ ] **Test names** clearly describe what is tested
- [ ] **Happy path** is tested
- [ ] **Edge cases** are covered
- [ ] **Error scenarios** are handled
- [ ] **CMakeLists.txt** is updated with new test names
- [ ] **Documentation** is updated if new category added
- [ ] **Tests pass** in CI

## Detailed Review Process

### Step 1: Identify UI Changes

Look for changes in these areas:

```
src/ui/MainWindow.cpp        # Menu actions, dialogs, window management
src/ui/MainWindow.Search.cpp # Find/Replace dialogs
src/ui/MainWindow.FileIO.cpp # File open/save dialogs
src/ui/MainWindow.Settings.cpp # Settings persistence
src/ui/TextEditor.cpp        # Editor interactions
src/ui/PrintSupport.cpp      # Print dialogs
```

**Action**: If any of these files are modified, UI tests are likely required.

### Step 2: Verify Test Files Updated

Check that appropriate test files are modified:

```
tests/smoke/MainWindowSmokeTests.h  # Test method declarations
tests/smoke/smoke.cpp               # Test implementations  
tests/CMakeLists.txt                # Test registration
```

**Red flags**:
- ❌ UI code changed but no test files updated
- ❌ New feature added without corresponding test
- ❌ Dialog added without invocation test

### Step 3: Review Test Coverage by Feature Type

Use this matrix to verify coverage:

#### For New Dialogs

Required tests:
- [ ] Dialog can be opened (invocation test)
- [ ] Dialog initial state is correct
- [ ] Dialog accepts valid input
- [ ] Dialog rejects invalid input
- [ ] Dialog cancel preserves state
- [ ] Dialog OK applies changes

**Example**:
```cpp
// Good: Tests dialog invocation
void testMyDialogInvocation()
{
    MainWindow window;
    window.setAutoDismissDialogsForTest(true);
    
    const int before = window.myDialogInvocationCountForTest();
    QMetaObject::invokeMethod(&window, "handleMyDialog");
    QTRY_COMPARE(window.myDialogInvocationCountForTest(), before + 1);
}

// Even better: Also tests dialog state
void testMyDialogState()
{
    MainWindow window;
    // Test dialog pre-populates with current value
    // Test dialog validates input range
    // Test dialog applies changes
}
```

#### For New Menu Actions

Required tests:
- [ ] Action exists in menu
- [ ] Action has keyboard shortcut (if applicable)
- [ ] Action is enabled/disabled appropriately
- [ ] Action triggers correct handler
- [ ] Action updates UI correctly

**Example**:
```cpp
void testMyMenuAction()
{
    MainWindow window;
    
    // Verify action exists
    auto* action = window.findActionByName("myAction");
    QVERIFY(action);
    
    // Verify shortcut
    QCOMPARE(action->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_M));
    
    // Verify enabled state
    QVERIFY(action->isEnabled());
    
    // Trigger and verify result
    action->trigger();
    QTRY_VERIFY(expectedStateChanged());
}
```

#### For Status Bar Changes

Required tests:
- [ ] New status bar widgets are created
- [ ] Widgets display correct initial state
- [ ] Widgets update on relevant events
- [ ] Tooltips are present and helpful

**Example**:
```cpp
void testMyStatusBarWidget()
{
    MainWindow window;
    auto* widget = window.findChild<QLabel*>("myWidget");
    QVERIFY(widget);
    QVERIFY(!widget->text().isEmpty());
    
    // Trigger update
    window.handleSomeAction();
    QTRY_VERIFY(widget->text().contains("updated"));
}
```

#### For Keyboard Shortcuts

Required tests:
- [ ] Shortcut is registered
- [ ] Shortcut executes correct action
- [ ] Shortcut works in appropriate contexts

**Example**:
```cpp
void testMyShortcut()
{
    MainWindow window;
    window.show();
    
    auto* editor = window.editorForTest();
    const QString before = editor->toPlainText();
    
    // Execute shortcut
    QTest::keySequence(&window, QKeySequence(Qt::CTRL | Qt::Key_M));
    
    QTRY_VERIFY(editor->toPlainText() != before);
}
```

#### For Action State Management

Required tests:
- [ ] Action disabled when not applicable
- [ ] Action enabled when applicable
- [ ] State updates on relevant events

**Example**:
```cpp
void testCopyActionState()
{
    MainWindow window;
    auto* editor = window.editorForTest();
    auto* copyAction = window.copyActionForTest();
    
    // No selection - should be disabled
    editor->clear();
    editor->textCursor().clearSelection();
    QTRY_VERIFY(!copyAction->isEnabled());
    
    // With selection - should be enabled
    editor->insertPlainText("Test");
    editor->selectAll();
    QTRY_VERIFY(copyAction->isEnabled());
}
```

### Step 4: Check Test Quality

Verify tests follow best practices:

#### Good Test Names

✅ **Good**: `testFindDialogOpensOnCtrlF()`
✅ **Good**: `testSaveActionDisabledWhenUnmodified()`
✅ **Good**: `testZoomResetReturnsTo100Percent()`

❌ **Bad**: `testDialog()`
❌ **Bad**: `testFeature1()`
❌ **Bad**: `test1()`

#### Proper Assertions

✅ **Good**:
```cpp
QVERIFY(action->isEnabled());
QCOMPARE(editor->zoomPercentage(), 100);
QTRY_VERIFY(dialog->isVisible());
```

❌ **Bad**:
```cpp
QVERIFY(true); // Meaningless
// Missing verification entirely
action->trigger(); // No QVERIFY of result
```

#### Resource Cleanup

✅ **Good**:
```cpp
void testFileOperation()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.filePath("test.txt");
    // ... test code ...
    // tempDir auto-cleans on scope exit
}
```

❌ **Bad**:
```cpp
void testFileOperation()
{
    QString path = "/tmp/test123.txt";
    // ... test code ...
    // File left behind!
}
```

#### Platform Independence

✅ **Good**:
```cpp
const QString path = resolveTestFile("sample.txt");
QVERIFY2(!path.isEmpty(), "sample.txt not found");
```

❌ **Bad**:
```cpp
const QString path = "/home/user/sample.txt"; // Linux-only!
```

### Step 5: Verify CMakeLists.txt Updates

Check that `tests/CMakeLists.txt` includes new tests:

```cmake
set(GNOTE_SMOKE_TEST_FUNCTIONS
    ...
    testExistingTest
    testMyNewTest  # <-- Should be added
)
```

**Red flag**: New test method exists but not in CMakeLists.txt

### Step 6: Check Documentation Updates

If adding a new test category or feature area:

- [ ] `UI_TEST_COVERAGE.md` updated with new requirements
- [ ] `QUICKSTART.md` updated with new test info
- [ ] Comments in test code explain non-obvious behavior

## Common Issues and How to Spot Them

### Issue 1: Tests Not Actually Testing Anything

**Symptom**: Test passes but doesn't verify behavior

```cpp
// BAD
void testDialog()
{
    MainWindow window;
    window.handleShowDialog();
    // No QVERIFY - test passes but proves nothing!
}
```

**Fix**: Add proper verification

```cpp
// GOOD
void testDialog()
{
    MainWindow window;
    window.setAutoDismissDialogsForTest(true);
    
    const int before = window.dialogCountForTest();
    window.handleShowDialog();
    QTRY_COMPARE(window.dialogCountForTest(), before + 1);
}
```

### Issue 2: Tests Depend on External State

**Symptom**: Tests fail intermittently or on different machines

```cpp
// BAD
void testFont()
{
    QFont font("Consolas", 12); // May not exist on all systems!
    editor->setFont(font);
    QCOMPARE(editor->font().family(), "Consolas"); // Fails if font missing
}
```

**Fix**: Use platform-independent approach

```cpp
// GOOD
void testFont()
{
    const QFont initialFont = editor->font();
    QVERIFY(!initialFont.family().isEmpty());
    QVERIFY(initialFont.pointSizeF() > 0);
    
    // Test font size change, not specific family
    const qreal newSize = initialFont.pointSizeF() + 2;
    QFont newFont = initialFont;
    newFont.setPointSizeF(newSize);
    editor->setFont(newFont);
    QCOMPARE(editor->font().pointSizeF(), newSize);
}
```

### Issue 3: Missing Edge Case Coverage

**Symptom**: Test only covers happy path

```cpp
// INCOMPLETE
void testGoToLine()
{
    window.goToLine(5);
    QCOMPARE(cursor.blockNumber(), 4); // Only tests valid input
}
```

**Fix**: Add edge case tests

```cpp
// COMPLETE
void testGoToLine()
{
    // Happy path
    window.goToLine(5);
    QCOMPARE(cursor.blockNumber(), 4);
    
    // Edge cases
    window.goToLine(1); // First line
    QCOMPARE(cursor.blockNumber(), 0);
    
    window.goToLine(document->blockCount()); // Last line
    QCOMPARE(cursor.blockNumber(), document->blockCount() - 1);
    
    // Invalid input (test error handling)
    window.goToLine(0); // Should handle gracefully
    window.goToLine(999999); // Beyond document
}
```

### Issue 4: Tests Don't Clean Up

**Symptom**: Test side effects affect other tests

```cpp
// BAD
void testSettings()
{
    QSettings settings;
    settings.setValue("test/key", "value");
    // ... test code ...
    // Settings persist to other tests!
}
```

**Fix**: Clean up in test or use isolated settings

```cpp
// GOOD
void testSettings()
{
    QTemporaryDir tempDir;
    QSettings settings(tempDir.filePath("test.ini"), QSettings::IniFormat);
    
    settings.setValue("test/key", "value");
    // ... test code ...
    
    // Auto-cleaned when tempDir goes out of scope
}
```

## Test Coverage Metrics to Review

When reviewing, check these metrics in the PR:

1. **Test Count**: Does the number of new tests match the number of new features?
2. **Code Coverage**: Do tests exercise all new/modified code paths?
3. **Edge Cases**: Are boundary conditions tested?
4. **Error Paths**: Are error scenarios covered?

## Requesting Changes

### Template for Test Coverage Feedback

```markdown
**Missing Test Coverage**

The following UI changes need test coverage:

1. **New Dialog in `MainWindow::handleMyDialog()`**
   - [ ] Add `testMyDialogInvocation()` to verify dialog opens
   - [ ] Add test hook: `int myDialogInvocationCountForTest()` 
   - [ ] Update `CMakeLists.txt` to include new test

2. **New Menu Action "My Action"**
   - [ ] Add `testMyActionExists()` to verify action in menu
   - [ ] Add `testMyActionShortcut()` to verify Ctrl+M shortcut
   - [ ] Add `testMyActionBehavior()` to verify action effect

3. **Status Bar Widget Changes**
   - [ ] Add `testMyWidgetUpdates()` to verify widget updates

Please refer to `tests/UI_TEST_COVERAGE.md` for requirements and examples.
```

## Approving PRs

Before approving a UI change PR, verify:

- ✅ All UI changes have corresponding tests
- ✅ Tests are well-named and documented
- ✅ Tests follow existing patterns
- ✅ CMakeLists.txt updated
- ✅ Tests pass (verified in CI)
- ✅ No test quality issues

**Only approve when test coverage is complete.**

## Resources for Contributors

Direct contributors to:

- `tests/UI_TEST_COVERAGE.md` - Complete requirements
- `tests/UI_TEST_ANALYSIS.md` - Coverage analysis
- `tests/QUICKSTART.md` - How to run tests
- `tests/smoke/smoke.cpp` - Example implementations

## Summary

Comprehensive UI test coverage is non-negotiable. Every UI feature must be tested. Use this checklist to ensure PRs meet the required standards:

1. ✅ Tests exist for all UI changes
2. ✅ Tests are high quality (proper assertions, cleanup, platform-independent)
3. ✅ Edge cases and error paths covered
4. ✅ CMakeLists.txt updated
5. ✅ Documentation updated (if needed)
6. ✅ Tests pass in CI

**When in doubt, request more tests. It's easier to add tests during PR review than to retrofit them later.**

## Questions?

Refer to:
- `CONTRIBUTING.md` for general guidelines
- `tests/UI_TEST_COVERAGE.md` for detailed requirements
- Existing tests in `tests/smoke/` for examples

---

*This document is part of the UI Test Improvements initiative. All code reviews must verify test coverage compliance.*
