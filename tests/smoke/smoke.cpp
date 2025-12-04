#include <QtWidgets/QApplication>

#include <QtCore/QByteArray>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QPoint>
#include <QtCore/QSettings>
#include <QtCore/QSize>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>
#include <QStringList>
#include <QtTest/QTest>

#include <QtGui/QAction>
#include <QtGui/QFont>
#include <QtGui/QTextCursor>
#include <QtGui/QTextOption>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QScrollBar>
#include <QStringConverter>

#include "ui/MainWindow.h"
#include "ui/TextEditor.h"

#include "MainWindowSmokeTests.h"

using namespace GnotePad::ui;

MainWindowSmokeTests::MainWindowSmokeTests(QObject* parent)
    : QObject(parent)
{}

void MainWindowSmokeTests::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void MainWindowSmokeTests::testLaunchShowsWindow()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QVERIFY(window.windowTitle().contains(QStringLiteral("GnotePad")));
}

void MainWindowSmokeTests::testWindowStateTransitions()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());

    const QPoint targetPos(40, 60);
    window.move(targetPos);
    QApplication::processEvents();
    QCOMPARE(window.pos(), targetPos);

    const QSize targetSize(640, 420);
    window.resize(targetSize);
    QApplication::processEvents();
    QCOMPARE(window.size(), targetSize);

    window.showMinimized();
    QTRY_VERIFY(window.windowState().testFlag(Qt::WindowMinimized));

    window.showMaximized();
    QTRY_VERIFY(window.windowState().testFlag(Qt::WindowMaximized));

    window.showNormal();
    QTRY_VERIFY(!window.windowState().testFlag(Qt::WindowMinimized));
    QTRY_VERIFY(!window.windowState().testFlag(Qt::WindowMaximized));
}

void MainWindowSmokeTests::testDefaultsWithoutSettings()
{
    {
        QSettings settings;
        const QString iniPath = settings.fileName();
        settings.clear();
        settings.sync();
        if(!iniPath.isEmpty())
        {
            QFile::remove(iniPath);
            QFile::remove(iniPath + QStringLiteral(".lock"));
        }
    }

    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    QVERIFY(window.recentFilesForTest().isEmpty());
    auto* menu = window.recentFilesMenuForTest();
    QVERIFY(menu);
    const auto actions = menu->actions();
    QVERIFY(!actions.isEmpty());
    QVERIFY(!actions.first()->isEnabled());

    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf8);
    QVERIFY(!window.currentBomForTest());

    QVERIFY(editor->lineNumbersVisible());
    QCOMPARE(editor->wordWrapMode(), QTextOption::NoWrap);
    QCOMPARE(editor->tabSizeSpaces(), 4);
    QCOMPARE(editor->zoomPercentage(), 100);

    const QFont font = editor->font();
    QVERIFY(!font.family().isEmpty());
    QVERIFY(font.pointSizeF() > 0.0);

    QCOMPARE(window.windowState(), Qt::WindowNoState);

    auto* statusBar = window.findChild<QStatusBar*>();
    QVERIFY(statusBar);
    QVERIFY(statusBar->isVisible());
}

void MainWindowSmokeTests::testHandlesCorruptSettings()
{
    const QString iniPath = [&]() {
        QSettings settings;
        const QString fileName = settings.fileName();
        settings.clear();
        settings.setValue("window/posX", -9999);
        settings.setValue("window/posY", -9999);
        settings.setValue("window/width", -400);
        settings.setValue("window/height", 0);
        settings.setValue("editor/tabSizeSpaces", 64);
        settings.setValue("editor/zoomPercent", 9999);
        settings.setValue("editor/fontFamily", QString());
        settings.setValue("editor/fontPointSize", 512.0);

        QStringList recents;
        for(int i = 0; i < 20; ++i)
        {
            recents.append(QStringLiteral("/tmp/corrupt_file_%1.txt").arg(i));
        }
        settings.setValue("documents/recentFiles", recents);
        settings.sync();
        return fileName;
    }();

    {
        MainWindow window;
        window.show();
        QTRY_VERIFY(window.isVisible());

        auto* editor = window.editorForTest();
        QVERIFY(editor);

        QVERIFY(window.width() > 0);
        QVERIFY(window.height() > 0);

        QCOMPARE(editor->tabSizeSpaces(), 16);
        QCOMPARE(editor->zoomPercentage(), 500);

        const QFont font = editor->font();
        QVERIFY(font.pointSizeF() < 50.0);

        const auto recents = window.recentFilesForTest();
        QCOMPARE(recents.size(), 10);
        QCOMPARE(recents.first(), QStringLiteral("/tmp/corrupt_file_0.txt"));
        QCOMPARE(recents.last(), QStringLiteral("/tmp/corrupt_file_9.txt"));
    }

    {
        QSettings settings;
        settings.clear();
        settings.sync();
        if(!iniPath.isEmpty())
        {
            QFile::remove(iniPath);
            QFile::remove(iniPath + QStringLiteral(".lock"));
        }
    }
}

void MainWindowSmokeTests::testZoomActions()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());

    auto* editor = window.findChild<TextEditor*>();
    QVERIFY(editor);

    const int originalZoom = editor->zoomPercentage();

    QMetaObject::invokeMethod(&window, "handleZoomIn");
    QTRY_VERIFY(editor->zoomPercentage() > originalZoom);

    QMetaObject::invokeMethod(&window, "handleZoomOut");
    QTRY_COMPARE(editor->zoomPercentage(), originalZoom);

    QMetaObject::invokeMethod(&window, "handleZoomReset");
    QTRY_COMPARE(editor->zoomPercentage(), 100);
}

void MainWindowSmokeTests::testInsertTimeDate()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());

    auto* editor = window.findChild<TextEditor*>();
    QVERIFY(editor);

    editor->clear();
    QMetaObject::invokeMethod(&window, "handleInsertTimeDate");
    QTRY_VERIFY(!editor->toPlainText().isEmpty());
}

void MainWindowSmokeTests::testToggleLineNumbers()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());

    auto* editor = window.findChild<TextEditor*>();
    QVERIFY(editor);

    QVERIFY(editor->lineNumbersVisible());
    QMetaObject::invokeMethod(&window, "handleToggleLineNumbers", Q_ARG(bool, false));
    QTRY_VERIFY(!editor->lineNumbersVisible());

    QMetaObject::invokeMethod(&window, "handleToggleLineNumbers", Q_ARG(bool, true));
    QTRY_VERIFY(editor->lineNumbersVisible());
}

void MainWindowSmokeTests::testOpenSampleFile()
{
    const QString samplePath = resolveTestFile(QStringLiteral("sample68.htm"));
    QVERIFY2(!samplePath.isEmpty(), "sample68.htm not found in testfiles directory");

    MainWindow window;
    QVERIFY(window.testLoadDocument(samplePath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QTRY_VERIFY(!editor->toPlainText().isEmpty());
    QVERIFY(window.windowTitle().contains(QStringLiteral("sample68")));
}

void MainWindowSmokeTests::testLargeFileScrolling()
{
    const QString largePath = resolveTestFile(QStringLiteral("ulysses8.htm"));
    QVERIFY2(!largePath.isEmpty(), "ulysses8.htm not found in testfiles directory");

    MainWindow window;
    QVERIFY(window.testLoadDocument(largePath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    editor->moveCursor(QTextCursor::End);
    QApplication::processEvents();

    auto* scrollBar = editor->verticalScrollBar();
    QVERIFY(scrollBar);
    QCOMPARE(scrollBar->value(), scrollBar->maximum());
}

void MainWindowSmokeTests::testSaveAsWithEncoding()
{
    const QString samplePath = resolveTestFile(QStringLiteral("sample68.htm"));
    QVERIFY2(!samplePath.isEmpty(), "sample68.htm not found in testfiles directory");

    MainWindow window;
    QVERIFY(window.testLoadDocument(samplePath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    const QString originalContent = editor->toPlainText();

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString savePath = tempDir.filePath(QStringLiteral("utf16le.txt"));

    QVERIFY(window.testSaveDocumentWithEncoding(savePath, QStringConverter::Utf16LE, true));

    QFile savedFile(savePath);
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    const QByteArray bom = savedFile.read(2);
    QCOMPARE(bom, QByteArray::fromHex("fffe"));
    savedFile.close();

    editor->clear();
    QVERIFY(window.testLoadDocument(savePath));
    QCOMPARE(editor->toPlainText(), originalContent);
    QCOMPARE(window.currentEncodingForTest(), QStringConverter::Utf16LE);
    QVERIFY(window.currentBomForTest());
}

void MainWindowSmokeTests::testEncodingRoundTripVariants()
{
    const QString samplePath = resolveTestFile(QStringLiteral("sample68.htm"));
    QVERIFY2(!samplePath.isEmpty(), "sample68.htm not found in testfiles directory");

    MainWindow window;
    QVERIFY(window.testLoadDocument(samplePath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    editor->appendPlainText(QStringLiteral("\n[Encoding Variants]\n"));
    editor->appendPlainText(QString(2048, QLatin1Char('Z')));
    const QString baseline = editor->toPlainText();

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto verifyVariant = [&](const QString& fileName, QStringConverter::Encoding encoding, bool bom, const QByteArray& expectedBom) {
        const QString path = tempDir.filePath(fileName);
        editor->setPlainText(baseline);
        editor->document()->setModified(true);
        QVERIFY(window.testSaveDocumentWithEncoding(path, encoding, bom));

        QFile savedFile(path);
        QVERIFY(savedFile.open(QIODevice::ReadOnly));
        const int prefixLength = expectedBom.isEmpty() ? 3 : expectedBom.size();
        const QByteArray prefix = savedFile.read(prefixLength);
        savedFile.close();

        if(expectedBom.isEmpty())
        {
            QVERIFY(!prefix.startsWith(QByteArray::fromHex("efbbbf")));
            QVERIFY(!prefix.startsWith(QByteArray::fromHex("fffe")));
            QVERIFY(!prefix.startsWith(QByteArray::fromHex("feff")));
        }
        else
        {
            QCOMPARE(prefix.left(expectedBom.size()), expectedBom);
        }

        editor->clear();
        QVERIFY(window.testLoadDocument(path));
        QCOMPARE(editor->toPlainText(), baseline);
        QCOMPARE(window.currentEncodingForTest(), encoding);
        QCOMPARE(window.currentBomForTest(), !expectedBom.isEmpty());
    };

    verifyVariant(QStringLiteral("utf8_bom.txt"), QStringConverter::Utf8, true, QByteArray::fromHex("efbbbf"));
    verifyVariant(QStringLiteral("utf8_plain.txt"), QStringConverter::Utf8, false, QByteArray());
    verifyVariant(QStringLiteral("utf16be.txt"), QStringConverter::Utf16BE, true, QByteArray::fromHex("feff"));
}

void MainWindowSmokeTests::testFindNavigation()
{
    const QString findPath = resolveTestFile(QStringLiteral("findreplace.txt"));
    QVERIFY2(!findPath.isEmpty(), "findreplace.txt not found in testfiles directory");

    MainWindow window;
    QVERIFY(window.testLoadDocument(findPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    window.setSearchStateForTest(QStringLiteral("cat"), Qt::CaseInsensitive);
    editor->moveCursor(QTextCursor::Start);
    QMetaObject::invokeMethod(&window, "handleFindNext");
    QCOMPARE(editor->textCursor().selectedText(), QStringLiteral("cat"));
    QCOMPARE(editor->textCursor().blockNumber(), 0);

    editor->moveCursor(QTextCursor::End);
    QMetaObject::invokeMethod(&window, "handleFindNext");
    QCOMPARE(editor->textCursor().selectedText(), QStringLiteral("cat"));
    QCOMPARE(editor->textCursor().blockNumber(), 0);

    window.setSearchStateForTest(QStringLiteral("dog"), Qt::CaseSensitive);
    editor->moveCursor(QTextCursor::Start);
    QMetaObject::invokeMethod(&window, "handleFindNext");
    QCOMPARE(editor->textCursor().selectedText(), QStringLiteral("dog"));
    QCOMPARE(editor->textCursor().blockNumber(), 2);

    window.setSearchStateForTest(QStringLiteral("dog"), Qt::CaseInsensitive);
    editor->moveCursor(QTextCursor::End);
    QMetaObject::invokeMethod(&window, "handleFindPrevious");
    QCOMPARE(editor->textCursor().selectedText(), QStringLiteral("DOG"));
    QCOMPARE(editor->textCursor().blockNumber(), 3);

    editor->moveCursor(QTextCursor::Start);
    QMetaObject::invokeMethod(&window, "handleFindPrevious");
    QCOMPARE(editor->textCursor().selectedText(), QStringLiteral("DOG"));
    QCOMPARE(editor->textCursor().blockNumber(), 3);
}

void MainWindowSmokeTests::testReplaceOperations()
{
    const QString findPath = resolveTestFile(QStringLiteral("findreplace.txt"));
    QVERIFY2(!findPath.isEmpty(), "findreplace.txt not found in testfiles directory");

    MainWindow window;
    QVERIFY(window.testLoadDocument(findPath));

    auto* editor = window.editorForTest();
    QVERIFY(editor);

    window.setSearchStateForTest(QStringLiteral("dog"), Qt::CaseInsensitive, QStringLiteral("wolf"));
    editor->moveCursor(QTextCursor::Start);
    QVERIFY(window.testReplaceNext());
    const QString afterSingle = editor->toPlainText();
    QVERIFY(afterSingle.contains(QStringLiteral("beta wolf")));
    QVERIFY(afterSingle.contains(QStringLiteral("gamma dog dog")));

    window.setSearchStateForTest(QStringLiteral("cat"), Qt::CaseInsensitive, QStringLiteral("lynx"));
    const int replacedCats = window.testReplaceAll(QStringLiteral("cat"), QStringLiteral("lynx"));
    QCOMPARE(replacedCats, 2);
    const QString afterAll = editor->toPlainText();
    QVERIFY(afterAll.contains(QStringLiteral("lynx alpha")));
    QVERIFY(afterAll.contains(QStringLiteral("epsilon lynx")));
    QVERIFY(!afterAll.contains(QStringLiteral("cat")));
}

void MainWindowSmokeTests::testRecentFilesMenu()
{
    const QString firstPath = resolveTestFile(QStringLiteral("sample68.htm"));
    const QString secondPath = resolveTestFile(QStringLiteral("ulysses8.htm"));
    const QString thirdPath = resolveTestFile(QStringLiteral("findreplace.txt"));
    QVERIFY2(!firstPath.isEmpty(), "sample68.htm not found");
    QVERIFY2(!secondPath.isEmpty(), "ulysses8.htm not found");
    QVERIFY2(!thirdPath.isEmpty(), "findreplace.txt not found");

    MainWindow window;
    QVERIFY(window.testLoadDocument(firstPath));
    QVERIFY(window.testLoadDocument(secondPath));
    QVERIFY(window.testLoadDocument(thirdPath));

    auto recents = window.recentFilesForTest();
    QCOMPARE(recents.size(), 3);
    QCOMPARE(recents.at(0), QFileInfo(thirdPath).absoluteFilePath());
    QCOMPARE(recents.at(1), QFileInfo(secondPath).absoluteFilePath());
    QCOMPARE(recents.at(2), QFileInfo(firstPath).absoluteFilePath());

    QVERIFY(window.testLoadDocument(secondPath));
    recents = window.recentFilesForTest();
    QCOMPARE(recents.size(), 3);
    QCOMPARE(recents.at(0), QFileInfo(secondPath).absoluteFilePath());
    QCOMPARE(recents.at(1), QFileInfo(thirdPath).absoluteFilePath());

    auto* menu = window.recentFilesMenuForTest();
    QVERIFY(menu);
    const auto actions = menu->actions();
    QVERIFY(!actions.isEmpty());
    int indexedEntry = 0;
    for(QAction* action : actions)
    {
        if(!action || !action->data().isValid())
        {
            continue;
        }
        QVERIFY(indexedEntry < recents.size());
        QCOMPARE(action->data().toString(), recents.at(indexedEntry));
        ++indexedEntry;
    }
    QCOMPARE(indexedEntry, recents.size());

    QMetaObject::invokeMethod(&window, "handleClearRecentFiles");
    QVERIFY(window.recentFilesForTest().isEmpty());

    const auto clearedActions = menu->actions();
    QVERIFY(!clearedActions.isEmpty());
    QVERIFY(!clearedActions.first()->isEnabled());
}

void MainWindowSmokeTests::testDestructivePrompts()
{
    const QString samplePath = resolveTestFile(QStringLiteral("sample68.htm"));
    QVERIFY2(!samplePath.isEmpty(), "sample68.htm not found");

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("scratch.txt"));

    auto resetTempFile = [&]() {
        QFile::remove(tempFile);
        QVERIFY(QFile::copy(samplePath, tempFile));
    };

    resetTempFile();

    MainWindow window;
    auto* editor = window.editorForTest();
    QVERIFY(editor);

    const auto stageDocument = [&](const QString& marker) {
        QVERIFY(window.testLoadDocument(tempFile));
        editor->moveCursor(QTextCursor::End);
        editor->insertPlainText(marker);
        editor->document()->setModified(true);
        QVERIFY(editor->document()->isModified());
    };

    const auto fileContents = [&]() -> QString {
        QFile file(tempFile);
        if(!file.open(QIODevice::ReadOnly))
        {
            QTest::qFail("Unable to open temp file", __FILE__, __LINE__);
            return {};
        }
        return QString::fromUtf8(file.readAll());
    };

    // Save branch should persist edits and reset document
    stageDocument(QStringLiteral("\n[SAVE]"));
    window.enqueueDestructivePromptResponseForTest(QMessageBox::Save);
    QMetaObject::invokeMethod(&window, "handleNewFile");
    QTRY_VERIFY(editor->toPlainText().isEmpty());
    QVERIFY(!editor->document()->isModified());
    QVERIFY(fileContents().contains(QStringLiteral("[SAVE]")));

    // Discard branch should drop edits without touching disk
    resetTempFile();
    stageDocument(QStringLiteral("\n[DISCARD]"));
    window.enqueueDestructivePromptResponseForTest(QMessageBox::Discard);
    QMetaObject::invokeMethod(&window, "handleNewFile");
    QTRY_VERIFY(editor->toPlainText().isEmpty());
    QVERIFY(!fileContents().contains(QStringLiteral("[DISCARD]")));

    // Cancel branch should leave edits and modification flag intact
    resetTempFile();
    stageDocument(QStringLiteral("\n[CANCEL]"));
    window.enqueueDestructivePromptResponseForTest(QMessageBox::Cancel);
    QMetaObject::invokeMethod(&window, "handleNewFile");
    QVERIFY(editor->document()->isModified());
    QVERIFY(editor->toPlainText().contains(QStringLiteral("[CANCEL]")));
    QVERIFY(!fileContents().contains(QStringLiteral("[CANCEL]")));
}

void MainWindowSmokeTests::testShortcutCommands()
{
    const QString samplePath = resolveTestFile(QStringLiteral("sample68.htm"));
    QVERIFY2(!samplePath.isEmpty(), "sample68.htm not found");

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString tempFile = tempDir.filePath(QStringLiteral("shortcut.txt"));
    QFile::remove(tempFile);
    QVERIFY(QFile::copy(samplePath, tempFile));

    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    window.setAutoDismissDialogsForTest(true);

    auto* editor = window.editorForTest();
    QVERIFY(editor);
    QVERIFY(window.testLoadDocument(tempFile));
    editor->setFocus();
    QCoreApplication::processEvents();

    const auto readFile = [&]() -> QString {
        QFile file(tempFile);
        if(!file.open(QIODevice::ReadOnly))
        {
            QTest::qFail("Unable to read shortcut save file", __FILE__, __LINE__);
            return {};
        }
        return QString::fromUtf8(file.readAll());
    };

    // Ctrl+S should save current document without prompting if file path is known
    editor->moveCursor(QTextCursor::End);
    editor->insertPlainText(QStringLiteral("\n[ShortcutSave]"));
    QTest::keySequence(&window, QKeySequence::Save);
    QTRY_VERIFY(!editor->document()->isModified());
    QTRY_VERIFY(readFile().contains(QStringLiteral("[ShortcutSave]")));

    // Ensure Find shortcut is registered and dispatches handler
    auto* findAction = window.findActionForTest();
    QVERIFY(findAction);
    QCOMPARE(findAction->shortcut(), QKeySequence::Find);
    const int findCountBefore = window.findDialogInvocationCountForTest();
    findAction->trigger();
    QTRY_COMPARE(window.findDialogInvocationCountForTest(), findCountBefore + 1);

    // Ensure Replace shortcut is registered and dispatches handler
    auto* replaceAction = window.replaceActionForTest();
    QVERIFY(replaceAction);
    QCOMPARE(replaceAction->shortcut(), QKeySequence::Replace);
    const int replaceCountBefore = window.replaceDialogInvocationCountForTest();
    replaceAction->trigger();
    QTRY_COMPARE(window.replaceDialogInvocationCountForTest(), replaceCountBefore + 1);

    // F5 should insert time/date stamp via existing action
    auto* timeDateAction = window.timeDateActionForTest();
    QVERIFY(timeDateAction);
    QCOMPARE(timeDateAction->shortcut(), QKeySequence(Qt::Key_F5));
    const int beforeLength = editor->toPlainText().length();
    timeDateAction->trigger();
    QTRY_VERIFY(editor->toPlainText().length() > beforeLength);
}

int main(int argc, char** argv)
{
    if(!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
    {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi, true);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GnotePadTests"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("tests.gnotepad.app"));
    QCoreApplication::setApplicationName(QStringLiteral("GnotePadSmokeTests"));
    QSettings::setDefaultFormat(QSettings::IniFormat);

    MainWindowSmokeTests tc;
    return QTest::qExec(&tc, argc, argv);
}

QString MainWindowSmokeTests::resolveTestFile(const QString& name) const
{
    QDir dir(QCoreApplication::applicationDirPath());
    if(!dir.cd(QStringLiteral("testfiles")))
    {
        return {};
    }

    const QString fullPath = dir.absoluteFilePath(name);
    if(QFileInfo::exists(fullPath))
    {
        return fullPath;
    }
    return {};
}
